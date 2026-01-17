#pragma once

#include "sat.h"
#include "dlx.h"
#include "cloud.h"

template<typename grid>
bool checkPeriodicRectangle(const Cloud<grid>& cloud, size_t w, size_t h)
{
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;

	if (grid::grid_type != OMINO) {
		return false;
	}

	size_t num_orientations = cloud.orientations_.size();

	// Number of rows: w * h * num_orientations
	// Number of columns: w * h
	// All columns required

	size_t stride = w * h;
	std::vector<bool> bgrid(stride * stride * num_orientations);

	size_t ri = 0;

	for (size_t dy = 0; dy < h; ++dy) {
		for (size_t dx = 0; dx < w; ++dx) {
			point_t dpos {(coord_t)dx, (coord_t)dy};

			for (const auto& o: cloud.orientations_) {
				for (const auto& pt: o.shape_) {
					// Row ri uses the column given by point pt translated by dx, dy
					size_t x = (pt.x_ + dx) % w;
					size_t y = (pt.y_ + dy) % h;
					bgrid[ri + y * w + x] = true;
				}
				ri += stride;
			}
		}
	}

	DLXMatrix m(w * h * num_orientations, w * h, w * h, 
		[&bgrid, stride](size_t r, size_t c) {return bgrid[r * stride + c];});
	bool solved = false;
	m.countSolutions(nullptr, 
		[&solved](const std::vector<size_t>& soln) {solved = true; return false;});
	return solved;
}

namespace Periodic {

const bool DEBUG = false;

template<typename grid>
struct cell_info {
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;

	var_id id_;
	point_t unit_;
	
	cell_info()
	{}

	cell_info(var_id id, const point_t& unit)
		: id_ {id}
		, unit_ {unit}
	{}
};

};

template<typename grid>
class PeriodicSolver {
public:
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;
	using xform_t = typename grid::xform_t;

	using cell_info_t = Periodic::cell_info<grid>;

	PeriodicSolver(const Shape<grid>& shape, size_t w, size_t h)
		: shape_ {shape}
		, w_ {w}
		, h_ {h}
	{
		// FIXME -- take advantage of grid::num_tile_shapes and 
		// grid::getTileShape to filter out shapes that definitely
		// can't tile the plane (because they don't have cells in 
		// the correct proportion).

		next_var_ = 0;
		
		unit_vars_ = new var_id[w_ * h_];
		for (size_t idx = 0; idx < w_ * h_; ++idx) {
			unit_vars_[idx] = declareVariable();

			if (Periodic::DEBUG) {
				std::cerr << "[" << unit_vars_[idx] << "]: Unit at <"
					<< (idx%w_) << ", " << (idx/w_) << ">" << std::endl;
			}
		}

		h_vars_ = new var_id[w_];
		for (size_t idx = 0; idx < w_; ++idx) {
			h_vars_[idx] = declareVariable();

			if (Periodic::DEBUG) {
				std::cerr << "[" << h_vars_[idx] << "]: " << idx 
					<< " is last V1 stripe" << std::endl;
			}
		}

		v_vars_ = new var_id[h_];
		for (size_t idx = 0; idx < h_; ++idx) {
			v_vars_[idx] = declareVariable();

			if (Periodic::DEBUG) {
				std::cerr << "[" << v_vars_[idx] << "]: " << idx 
					<< " is last V2 stripe" << std::endl;
			}
		}
	}

	~PeriodicSolver()
	{
		delete [] unit_vars_;
		delete [] h_vars_;
		delete [] v_vars_;
	}

	bool solve(std::vector<xform_t>* patch = nullptr);
	
private:
	var_id declareVariable();
	void buildCells();
	void buildTiles();
	void getTilesAtCell(const point_t& cp, 
		std::vector<std::pair<xform_t, var_id>>& ret) const;
	bool hasSelfOverlap(const point_t& v) const;

	void addSubgridClauses(CMSat::SATSolver& solver) const;
	void addOccupancyClauses(CMSat::SATSolver& solver) const;
	void addWraparoundClauses(CMSat::SATSolver& solver) const;

	void debugSolution(CMSat::SATSolver& solver) const;

	Shape<grid> shape_;

	size_t w_;
	size_t h_;
	var_id next_var_;

	var_id *unit_vars_;
	var_id *h_vars_;
	var_id *v_vars_;

	xform_map<coord_t, var_id> tilemap_;
	point_map<coord_t, cell_info_t> cellmap_;
};

template<typename grid>
var_id PeriodicSolver<grid>::declareVariable()
{
	var_id ret = next_var_;
	++next_var_;
	return ret;
}

template<typename grid>
bool PeriodicSolver<grid>::solve(std::vector<xform_t>* patch)
{
	buildCells();
	buildTiles();

	CMSat::SATSolver solver;
	solver.new_vars(next_var_);

	addSubgridClauses(solver);
	addOccupancyClauses(solver);
	addWraparoundClauses(solver);

	bool ret = solver.solve() == CMSat::l_True;

	if (ret && patch) {
		const std::vector<CMSat::lbool>& model = solver.get_model();
		for (auto v: tilemap_) {
			if (model[v.second] == CMSat::l_True) {
				patch->push_back(v.first);
			}
		}
	}

	if (Periodic::DEBUG) {
		std::cerr << "-----" << std::endl;
		if (ret) {
			std::cerr << "SOLVED" << std::endl;
			debugSolution(solver);
		} else {
			std::cerr << "NO SOLUTION" << std::endl;
		}
	}

	return ret;
}

template<typename grid>
void PeriodicSolver<grid>::buildCells() {
	point_t row_start {0, 0};

	for (size_t y = 0; y < h_; ++y) {
		point_t O = row_start;
		for (size_t x = 0; x < w_; ++x) {
			point_t u {(int16_t)x, (int16_t)y};
			for (const auto& p: grid::origins) {
				point_t op = O + p;
				var_id v = declareVariable();
				cellmap_[op] = cell_info_t {v, u};

				if (Periodic::DEBUG) {
					std::cerr << "[" << v << "]: Cell " << op
						<< " in unit " << u << std::endl;
				}
			}
			O = O + grid::translationV1;
		}
		row_start = row_start + grid::translationV2;
	}
}

template<typename grid>
void PeriodicSolver<grid>::buildTiles() 
{
	for (const auto& ci: cellmap_) {
		const point_t& cp = ci.first;

		for (const auto& T: grid::orientations) {
			for (const auto& p: shape_) {
				point_t tp = T * p;
				if (grid::translatable(tp, cp)) {
					// This is a legitimate cell placement.  But it might
					// have been placed already.
					xform_t NT = T.translate(cp - tp);
					if (tilemap_.find(NT) != tilemap_.end()) {
						continue;
					}

					var_id tv = declareVariable();
					tilemap_[NT] = tv;

					if (Periodic::DEBUG) {
						std::cerr << "[" << tv << "]: Tile "
							<< NT;
						for (const auto& pp: shape_) {
							std::cerr << " " << (NT * pp);
						}
						std::cerr << std::endl;
					}
				}
			}
		}
	}
}

template<typename grid>
void PeriodicSolver<grid>::getTilesAtCell(
	const point_t& cp, std::vector<std::pair<xform_t, var_id>>& ret) const
{
	for (const auto& T: grid::orientations) {
		for (const auto& p: shape_) {
			point_t tp = T * p;
			if (grid::translatable(tp, cp)) {
				xform_t NT = T.translate(cp - tp);
				auto i = tilemap_.find(NT);
				if (i != tilemap_.end()) {
					ret.push_back(*i);
				}
			}
		}
	}
}

template<typename grid>
bool PeriodicSolver<grid>::hasSelfOverlap(const point_t& v) const
{
	Shape<grid> trs {shape_};
	trs.translate(v);
	return shape_.intersects(trs);
}

template<typename grid>
void PeriodicSolver<grid>::addSubgridClauses(CMSat::SATSolver& solver) const
{
	std::vector<CMSat::Lit> cl;
	cl.resize(2);

	// Every column forces the one to its left
	for (size_t x = 1; x < w_; ++x) {
		cl[0] = neg(h_vars_[x]);
		cl[1] = pos(h_vars_[x - 1]);
		solver.add_clause(cl);
	}

	// Every row forces the one below it
	for (size_t y = 1; y < h_; ++y) {
		cl[0] = neg(v_vars_[y]);
		cl[1] = pos(v_vars_[y - 1]);
		solver.add_clause(cl);
	}

	// Every combination of h and v forces the corresponding unit, 
	// and vice versa
	std::vector<CMSat::Lit> cl3;
	cl3.resize(3);
	for (size_t y = 0; y < h_; ++y) {
		for (size_t x = 0; x < w_; ++x) {
			var_id hv = h_vars_[x];
			var_id vv = v_vars_[y];
			var_id uv = unit_vars_[y * w_ + x];

			cl[0] = neg(uv);

			// A unit forces its column
			cl[1] = pos(hv);
			solver.add_clause(cl);

			// A unit forces its row
			cl[1] = pos(vv);
			solver.add_clause(cl);

			// A column, row pair forces its unit
			cl3[0] = neg(hv);
			cl3[1] = neg(vv);
			cl3[2] = pos(uv);
			solver.add_clause(cl3);
		}
	}

	// U00 must be used
	cl.resize(1);
	cl[0] = pos(unit_vars_[0]);
	solver.add_clause(cl);

	cl.resize(2);

	// Exclude illegal v1 translations
	point_t v1 = grid::translationV1;
	for (size_t idx = 1; idx < shape_.size(); ++idx) {
		if (hasSelfOverlap(v1)) {
			// We're not allowed to have h(idx-1) true and h(idx) false,
			// i.e., this can't be the changeover point.
			cl[0] = neg(h_vars_[idx-1]);
			cl[1] = pos(h_vars_[idx]);
			solver.add_clause(cl);
		}
		v1 = v1 + grid::translationV1;
	}

	// Exclude illegal v2 translations
	point_t v2 = grid::translationV2;
	for (size_t idx = 1; idx < shape_.size(); ++idx) {
		if (hasSelfOverlap(v2)) {
			cl[0] = neg(v_vars_[idx-1]);
			cl[1] = pos(v_vars_[idx]);
			solver.add_clause(cl);
		}
		v2 = v2 + grid::translationV2;
	}
}

template<typename grid>
void PeriodicSolver<grid>::addOccupancyClauses(CMSat::SATSolver& solver) const
{
	std::vector<CMSat::Lit> cl;
	cl.resize(2);

	// A unit forces all its cells.
	// (Don't make a cell force its unit)
	for (const auto& pa: cellmap_) {
		const cell_info_t& ci = pa.second;

		var_id cv = ci.id_;
		var_id uv = unit_vars_[ci.unit_.y_ * w_ + ci.unit_.x_];

/*
		std::cerr << "Cell " << pa.first << " [" << cv << "] <=> unit "
			<< ci.unit_ << " [" << uv << "]" << std::endl;
			*/

/*
		cl[0] = neg(cv);
		cl[1] = pos(uv);
		solver.add_clause(cl);
		*/

		cl[0] = neg(uv);
		cl[1] = pos(cv);
		solver.add_clause(cl);
	}
	
	// Keep track of cells outside the cellmap, where we'd also like
	// to suppress overlaps.
	point_set<coord_t> extra_cells;

	// Every tile forces all of its cells
	for (const auto& tinfo: tilemap_) {
		const xform_t& T = tinfo.first;
		var_id v = tinfo.second;
		cl[0] = neg(v);

		for (const auto& p: shape_) {
			point_t tp = T * p;
			const auto i = cellmap_.find(tp);
			if (i != cellmap_.end()) {
				cl[1] = pos(i->second.id_);
				solver.add_clause(cl);
			} else {
				extra_cells.insert(tp);
			}
		}
	}

	std::vector<std::pair<xform_t, var_id>> tiles_at_cell;
	std::vector<CMSat::Lit> tacl;

	for (const auto& pa: cellmap_) {
		const point_t& cp = pa.first;
		const cell_info_t& ci = pa.second;

		tiles_at_cell.clear();
		tacl.clear();
		tacl.push_back(neg(ci.id_));

		getTilesAtCell(cp, tiles_at_cell);
		for (auto tac: tiles_at_cell) {
			tacl.push_back(pos(tac.second));
		}
		solver.add_clause(tacl);

		for (size_t idx = 0; idx < tiles_at_cell.size(); ++idx) {
			cl[0] = neg(tiles_at_cell[idx].second);
			for (size_t jdx = 0; jdx < idx; ++jdx) {
				cl[1] = neg(tiles_at_cell[jdx].second);
				solver.add_clause(cl);
			}
		}
	}

	for (const auto& cp: extra_cells) {
		tiles_at_cell.clear();
		getTilesAtCell(cp, tiles_at_cell);

		for (size_t idx = 0; idx < tiles_at_cell.size(); ++idx) {
			cl[0] = neg(tiles_at_cell[idx].second);
			for (size_t jdx = 0; jdx < idx; ++jdx) {
				cl[1] = neg(tiles_at_cell[jdx].second);
				solver.add_clause(cl);
			}
		}
	}
}

template<typename grid>
void PeriodicSolver<grid>::addWraparoundClauses(CMSat::SATSolver& solver) const
{
	std::vector<std::pair<xform_t, var_id>> tiles_at_cell;
	std::vector<CMSat::Lit> cl;
	cl.resize(5);

	point_t v2 {0, 0};
	for (size_t y = 0; y < h_; ++y) {
		cl[0] = neg(v_vars_[y]);

		// Find all cells in the translation unit translated by y*V2
		xform_set<coord_t> handled;

		for (const auto& p: grid::origins) {
			point_t cp = p + v2;
			// Find all tiles that might occupy that cell
			tiles_at_cell.clear();
			getTilesAtCell(cp, tiles_at_cell);
			for (const auto& tac: tiles_at_cell) {
				if (handled.find(tac.first) != handled.end()) {
					// Seen this tile along the left edge at this y value.
					continue;
				}
				handled.insert(tac.first);
				cl[3] = neg(tac.second);

				// For every i, add a clause that causes wraparound at
				// that multiple of V1
				point v1 = grid::translationV1;
				for (size_t idx = 1; idx < w_; ++idx) {
					xform_t NT = tac.first.translate(v1);
					const auto i = tilemap_.find(NT);
					if (i == tilemap_.end()) {
						std::cerr << "Hmm 1" << std::endl;
					}

					cl[1] = neg(h_vars_[idx-1]);
					cl[2] = pos(h_vars_[idx]);
					cl[4] = pos(i->second);
					solver.add_clause(cl);

					v1 += grid::translationV1;
				}
			}
		}

		v2 += grid::translationV2;
	}

	point_t v1 {0, 0};
	for (size_t x = 0; x < w_; ++x) {
		cl[0] = neg(h_vars_[x]);

		// Find all cells in the translation unit translated by y*V2
		xform_set<coord_t> handled;

		for (const auto& p: grid::origins) {
			point_t cp = p + v1;
			// Find all tiles that might occupy that cell
			tiles_at_cell.clear();
			getTilesAtCell(cp, tiles_at_cell);
			for (const auto& tac: tiles_at_cell) {
				if (handled.find(tac.first) != handled.end()) {
					// Seen this tile along the left edge at this y value.
					continue;
				}
				handled.insert(tac.first);
				cl[3] = neg(tac.second);

				// For every j, add a clause that causes wraparound at
				// that multiple of V2
				point v2 = grid::translationV2;
				for (size_t jdx = 1; jdx < h_; ++jdx) {
					xform_t NT = tac.first.translate(v2);
					const auto i = tilemap_.find(NT);
					if (i == tilemap_.end()) {
						std::cerr << "Hmm 2" << std::endl;
					}

					cl[1] = neg(v_vars_[jdx-1]);
					cl[2] = pos(v_vars_[jdx]);
					cl[4] = pos(i->second);
					solver.add_clause(cl);

					v2 += grid::translationV2;
				}
			}
		}

		v1 += grid::translationV1;
	}
}

template<typename grid>
void PeriodicSolver<grid>::debugSolution(CMSat::SATSolver& solver) const
{
	const std::vector<CMSat::lbool>& model = solver.get_model();

	size_t total = 0;
	for (auto b: model) {
		if (b == CMSat::l_True) {
			++total;
		}
	}
	std::cerr << total << " true vars" << std::endl;

	std::cerr << "hvars:";
	for (size_t idx = 0; idx < w_; ++idx) {
		if (model[h_vars_[idx]] == CMSat::l_True) {
			std::cerr << " 1";
		} else {
			std::cerr << " 0";
		}
	}
	std::cerr << std::endl;

	std::cerr << "vvars:";
	for (size_t idx = 0; idx < h_; ++idx) {
		if (model[v_vars_[idx]] == CMSat::l_True) {
			std::cerr << " 1";
		} else {
			std::cerr << " 0";
		}
	}
	std::cerr << std::endl;

	std::cerr << "units:";
	for (size_t y = 0; y < h_; ++y) {
		for (size_t x = 0; x < w_; ++x) {
			if (model[unit_vars_[y * w_ + x]] == CMSat::l_True) {
				std::cerr << " [" << x << ", " << y << "]";
			}
		}
	}
	std::cerr << std::endl;

	std::cerr << "cells:";
	for (auto i: cellmap_) {
		if (model[i.second.id_] == CMSat::l_True) {
			std::cerr << " " << i.first;
		}
	}
	std::cerr << std::endl;

	std::cerr << "tiles:" << std::endl;
	for (auto i: tilemap_) {
		if (model[i.second] == CMSat::l_True) {
			std::cerr << "   [" << i.second << "] " << i.first;
			for (const auto& p: shape_) {
				std::cerr << " " << (i.first *p);
			}
			std::cerr << std::endl;
		}
	}
}
