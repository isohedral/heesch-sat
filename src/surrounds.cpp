#include <iostream>
#include <cstdint>
#include <sstream>
#include <map>
#include <bitset>

#include <vector>
#include <unordered_set>
#include <unordered_map>

#include "heesch.h"
#include "grid.h"
#include "tileio.h"
#include "cloud.h"

#include "dlx.h"

// Enumerate all surrounds of a given polyform.

using namespace std;

static Orientations ori = ALL;
static bool show = false;
static bool verbose = false;
static bool reduce = false;
static bool show_holes = true;

template<typename grid>
static bool describeNeighbours(TileInfo<grid>& tile)
{
	using coord_t = typename grid::coord_t;
	using xform_t = typename grid::xform_t;

	Cloud<grid> cloud {tile.getShape(), ori, true, reduce};
	cerr << cloud.orientations_.size() << " orientations" << endl;

	if (show) {
		xform_t id;
		for (const xform_t& T: cloud.adjacent_) {
			LabelledPatch<coord_t> patch = {{0, id}, {1, T}};
			tile.setInconclusive(patch);
			tile.write(cout);
		}
	} else {
		tile.write(cout);
		cout << cloud.adjacent_.size() << " adjacents" << endl;
	}

	return true;
}

template<typename grid>
struct SizeInfo {
	using coord_t = typename grid::coord_t;
	using patch_t = LabelledPatch<coord_t>;

	size_t no_holes {0};
	patch_t no_hole_patch {};
	size_t holes {0};
	patch_t hole_patch {};

	operator bool() const 
	{ return no_holes > 0 || holes > 0; }
};

template<typename grid>
static bool computeSurrounds(TileInfo<grid> & tile)
{
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;
	using xform_t = typename grid::xform_t;
	using bitgrid_t = bitgrid<128>;

	Cloud<grid> cloud {tile.getShape(), ori, true, reduce};
	// FIXME -- could abort early here if cloud reports that the
	// shape isn't surroundable.
	size_t sz = cloud.adjacent_.size();

	point_map<coord_t, size_t> cell_map;
	uint32_t num_cols = 0;
	vector<xform_t> shape_map;
	shape_map.reserve(sz);
	vector<SizeInfo<grid>> sizes;

	for (const auto & P : cloud.halo_) {
		cell_map[P] = num_cols++;
	}

	// TODO: can we combine the below two for loops?
	// need to know length of row beforehand...
	// or, modify so that every row is not necessarily the same length
	// but need to modify the constructor for DLX to accept numColumns
	for( const auto& T : cloud.adjacent_ ) {
		for( const auto& P : tile.getShape()) {
			point_t tp = T * P;
			
			if (cell_map.find(tp) != cell_map.end()) {
				continue;
			}
			cell_map[tp] = num_cols++;
		}

		shape_map.push_back(T);
	}

	vector<bool> dlx_matrix(sz * num_cols);
	for (size_t idx = 0; idx < sz; ++idx) {
		const xform_t& T = shape_map[idx];
		for (const auto& P: tile.getShape()) {
			point_t tp = T * P;
			size_t vidx = cell_map[tp];
			dlx_matrix[idx * num_cols + vidx] = true;
		}
	}
			
	// TODO -- accelerate this further by making the lookup completely implicit, so
	// that you don't ever need to construct a matrix of booleans.  Turns out that
	// doing so is a little annoying, and may not actually save memory.  Investigate.
	DLXMatrix dlx(sz, num_cols, cloud.halo_.size(),
		[&dlx_matrix, num_cols](size_t r, size_t c) {return dlx_matrix[r*num_cols + c];});

	Shape<grid> halo;
	Shape<grid> border;
	Shape<grid> shape = tile.getShape();
	shape.getHaloAndBorder( halo, border );	

	bitgrid_t halo_bits;
	size_t halo_size = 0;
	for( auto p : halo ) {
		halo_bits.set( p, 1 );
		halo_size++;
	}

	bitgrid_t bits;
	for( const auto& p : shape ) {
		bits.set( p, 1 );
	}

	// Pass by value so that we don't have to redo the above computatons
	auto process = [bits, halo_bits, halo_size, &shape, &shape_map, &sizes](const vector<size_t> & solution) mutable
	{
		point_t start;
		for (const auto & row : solution) {
			const auto& T = shape_map[row];

			for (const auto & p : shape) {
				point_t tp = T * p;
				bits.set(tp, 1);

				// Use get and set?
				if (halo_bits.get(tp)) {
					halo_bits.set(tp, 0);
					halo_size--;
				}

				for (const auto & pn : neighbours<grid> { tp }) {
					if (bits.get(pn)) continue;
					if (!halo_bits.get(pn)) {
						halo_bits.set(pn, 1);
						halo_size++;

						// this start cell might get deleted above
						start = pn;
					}
				}
			}
		}

		// somewhat hacky way to get start point because of above
		// assuming that the next cell we encounter after going all the way right
		// is a halo cell (which makes sense, IMO)
		while (bits.get(start)) {
			start.x_ += 1;
		}

		point_t stack[128];
		size_t size = 1;
		size_t num_visited = 0;
		stack[0] = start;
		halo_bits.set(start, 0);

		while( size > 0 ) {
			auto p = stack[--size];
			++num_visited;

			for( auto pn : edge_neighbours<grid> { p } ) {
				if (halo_bits.get(pn) == 0) continue;
				halo_bits.set(pn, 0);
				stack[size++] = pn;
			}
		}

		size_t ssz = solution.size();
		if (ssz + 1 > sizes.size()) {
			sizes.resize(ssz + 1);
		}

		SizeInfo<grid>& si = sizes[ssz];

		if (num_visited != halo_size) {
			++si.holes;
			if (show && si.hole_patch.empty()) {
				si.hole_patch.emplace_back(0, xform_t {});
				for (const auto& row : solution) {
					const auto& T = shape_map[row];
					si.hole_patch.emplace_back(1, T);
				}
			}
		} else {
			++si.no_holes;
			if (show && si.no_hole_patch.empty()) {
				si.no_hole_patch.emplace_back(0, xform_t {});
				for (const auto& row : solution) {
					const auto& T = shape_map[row];
					si.no_hole_patch.emplace_back(1, T);
				}
			}
		}

		return true;
	};

	dlx.countSolutions(nullptr, process);

	bool start = true;
	
	if (show) {
		for (size_t idx = 0; idx < sizes.size(); ++idx) {
			const SizeInfo<grid>& si = sizes[idx];

			if (si) {
				if (si.no_hole_patch.empty()) {
					if (show_holes) {
						tile.setInconclusive(si.hole_patch);
						tile.write(cout);
					}
				} else {
					tile.setInconclusive(si.no_hole_patch);
					tile.write(cout);
				}
			}
		}
	} else {
		tile.write(cout);

		for (size_t idx = 0; idx < sizes.size(); ++idx) {
			if (sizes[idx]) {
				if (verbose) {
					cout << "Size " << idx << ": "
						<< sizes[idx].no_holes << " without holes, "
						<< sizes[idx].holes << " with holes." << endl;
				} else {
					if (start) {
						start = false;
					} else {
						cout << " ";
					}
					cout << idx << ": " 
						<< sizes[idx].no_holes << "/" << sizes[idx].holes
						<< ";";
				}
			} 
		}
		if (!verbose) {
			cout << endl;
		}
	}

	return true;
}

// A cheap single-purpose algorithm that checks if a tile has a 3-surround.
// This could probably be sped up further, and doesn't guarantee that the
// surround is hole-free.  But these are already so rare that it's probably
// not really worth optimizing more.
template<typename grid>
static bool filter3Surround(TileInfo<grid> & tile)
{
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;
	using xform_t = typename grid::xform_t;
	using bits_t = bitset<256>;

	// Don't filter symmetries here,
	// so that we can invert matrices below.
	Cloud<grid> cloud {tile.getShape(), ori, true, true};
	vector<pair<xform_t, bits_t>> adjs;

	// Assign an index to each halo cell
	point_map<coord_t, size_t> cell_map;
	uint32_t num_cols = 0;
	for (const auto & P : cloud.halo_) {
		cell_map[P] = num_cols++;
	}

	// For each adjacent, figure out which halo cells it
	// occupies.
	for (const auto& T: cloud.adjacent_) {
		bits_t bits;

		for (const auto& p: tile.getShape()) {
			point_t tp = T * p;
			if (cell_map.find(tp) != cell_map.end()) {
				bits[cell_map[tp]] = true;	
			}
		}
	
		// cout << T << " -> " << bits << endl;
		adjs.emplace_back(T, std::move(bits));
	}

	for (size_t idx = 0; idx < adjs.size(); ++idx) {
		const auto& TA = adjs[idx].first;
		const auto& ba = adjs[idx].second;

		for (size_t jdx = 0; jdx < idx; ++jdx) {
			const auto& TB = adjs[jdx].first;
			const auto& bb = adjs[jdx].second;

			if ((ba & bb).any()) {
				// A and B overlap, no point in checking further.
				continue;
			}

			// May as well check for 2-surrounds, since it's basically
			// free...
			if ((ba | bb).count() == cloud.halo_.size()) {
				LabelledPatch<coord_t> patch;
				patch.emplace_back(0l, xform_t {});
				patch.emplace_back(1l, TA);
				patch.emplace_back(1l, TB);
				tile.setInconclusive(patch);
				tile.write(cout);
				continue;
			}

			for (size_t kdx = 0; kdx < jdx; ++kdx) { 
				const auto& TC = adjs[kdx].first;
				const auto& bc = adjs[kdx].second;

				if ((ba & bc).any() || (bb & bc).any()) {
					continue;
				}
				
				if ((ba | bb | bc).count() == cloud.halo_.size()) {
					LabelledPatch<coord_t> patch;
					patch.emplace_back(0l, xform_t {});
					patch.emplace_back(1l, TA);
					patch.emplace_back(1l, TB);
					patch.emplace_back(1l, TC);
					tile.setInconclusive(patch);
					tile.write(cout);
				}
			}
		}
	}

	return true;
}

int main( int argc, char **argv )
{
	bool neighs = false;
	bool three = false;

	for( int idx = 1; idx < argc; ++idx ) {
		if (!strcmp(argv[idx], "-show")) {
			show = true;
		} else if (!strcmp(argv[idx], "-translations")) {
			ori = TRANSLATIONS_ONLY;
		} else if (!strcmp(argv[idx], "-rotations")) {
			ori = TRANSLATIONS_ROTATIONS;
		} else if( !strcmp( argv[idx], "-neighbours" ) ) {
		    neighs = true;
		} else if( !strcmp( argv[idx], "-verbose" ) ) {
		    verbose = true;
		} else if( !strcmp( argv[idx], "-reduce" ) ) {
		    reduce = true;
		} else if( !strcmp( argv[idx], "-noreduce" ) ) {
		    reduce = false;
		} else if( !strcmp( argv[idx], "-three" ) ) {
		    three = true;
		} else if( !strcmp( argv[idx], "-noholes" ) ) {
		    show_holes = false;
		} else {
			cerr << "Unrecognized parameter \"" << argv[idx] << "\""
				<< endl;
			exit( 0 );
		}
	}

	if (three) { 
		processInputStream(cin, filter3Surround);
	} else if (neighs) {
		processInputStream(cin, describeNeighbours);
	} else {
		processInputStream( cin, computeSurrounds );
	}
	return 0;
}
