#pragma once

#include <vector>
#include <list>
#include <map>

#include <cryptominisat.h>

#include "cloud.h"
#include "holes.h"

using var_id = uint32_t;

template<typename grid>
struct tile_info
{
	using xform_t = typename grid::xform_t;

	tile_info( const xform_t& T, tile_index index )
		: T_ { T }
		, index_ { index }
		, vars_ {}
		, cells_ {}
	{}

	bool hasLevel( size_t level ) const
	{
		return vars_.find( level ) != vars_.end();
	}

	xform_t T_;
	tile_index index_;

	// The SAT variable used at each corona level accessible at this location.
	std::map<size_t,var_id> vars_;
	std::list<cell_index> cells_;
};

template<typename grid>
struct cell_info
{
	using point_t = typename grid::point_t;

	cell_info( const point_t& p, cell_index index )
		: pos_ { p }
		, index_ { index }
		, tiles_ {}
	{}

	point_t pos_;
	cell_index index_;
	var_id var_;

	std::list<tile_index> tiles_;
};

template<typename coord_t>
using Solution = std::vector<std::pair<size_t,xform<coord_t>>>;

template<typename grid>
class HeeschSolver
{
public:
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;
	using xform_t = typename grid::xform_t;

	HeeschSolver( const Shape<grid>& shape, Orientations ori = ALL );

	void increaseLevel();
	size_t getLevel() const
	{ return level_; }

	bool hasCorona( 
		bool get_solution, bool& has_holes, Solution<coord_t>& soln );
	void allCoronas( std::vector<Solution<coord_t>>& solns );
	bool isIsohedral( bool get_solution, Solution<coord_t>& soln );

	void debug( std::ostream& os ) const;

private:
	var_id declareVariable();
	tile_index getTile( const xform_t& T ) const;
	cell_index getCell( const point_t& p, bool create );
	tile_index createNewTile( const xform_t& T );

	var_id getShapeVariable( const xform_t& T, size_t level );
	bool getShapeVariable( const xform_t& T, size_t level, var_id& id ) const;
	bool hasCell( const point_t& p ) const;
	var_id getCellVariable( const point_t& p );
	var_id getCellVariable( const point_t& p ) const;

	void getClauses( CMSat::SATSolver& solv, bool allow_holes ) const;
	void getSolution(
		const CMSat::SATSolver& solv, Solution<coord_t>& ret ) const;
	void addHolesToLevel();
	void extendLevelWithTransforms( size_t lev, const xform_set<coord_t>& Ts );

	Shape<grid> shape_;
	Cloud<grid> cloud_;

	std::vector<tile_info<grid>> tiles_;
	std::vector<cell_info<grid>> cells_;

	xform_map<coord_t,tile_index> tile_map_;
	point_map<coord_t,cell_index> cell_map_;

	size_t level_;
	var_id next_var_;
};

template<typename grid, typename coord>
void debugSolution( 
	std::ostream& os, const Shape<grid>& shape, const Solution<coord>& soln )
{
	using point_t = typename grid::point_t;

	static char buf[2500];
	std::fill( buf, buf + 2500, ' ' );

	int xmin = 50;
	int ymin = 50;
	int xmax = 0;
	int ymax = 0;

	int count = 0;
	
	for( auto& i : soln ) {
		char ch = 'A' + count;

		for( auto p : shape ) {
			point_t tp = i.second * p;
			if( (tp.x_<-25) || (tp.x_>=25) || (tp.y_<-25) || (tp.y_>=25) ) {
				continue;
			}

			buf[50*(tp.y_+25) + (tp.x_+25)] = ch;
			xmin = std::min( xmin, int(tp.x_ + 25) );
			xmax = std::max( xmax, int(tp.x_ + 25) );
			ymin = std::min( ymin, int(tp.y_ + 25) );
			ymax = std::max( ymax, int(tp.y_ + 25) );
		}

		count = (count+1)%26;
	}

	for( size_t y = ymin; y <= ymax; ++y ) {
		for( size_t x = xmin; x <= xmax; ++x ) {
			os << buf[y*50+x];
		}
		os << std::endl;
	}
	os << std::endl;
}

template<typename grid>
HeeschSolver<grid>::HeeschSolver( const Shape<grid>& shape, Orientations ori )
	: shape_ { shape }
	, cloud_ { shape, ori }
	, tiles_ {}
	, cells_ {}
	, tile_map_ {}
	, cell_map_ {}
	, level_ { 0 }
	, next_var_ { 0 }
{
	// Create the 0th corona.
	getShapeVariable( grid::orientations[0], 0 );
}

template<typename grid>
var_id HeeschSolver<grid>::declareVariable()
{
	var_id ret = next_var_;
	++next_var_;
	return ret;
}

template<typename grid>
cell_index HeeschSolver<grid>::getTile( const xform_t& T ) const
{
	const auto& i = tile_map_.find( T );
	if( i != tile_map_.end() ) {
		return tiles_[ i->second ].index_;
	}

	return -1;
}

template<typename grid>
cell_index HeeschSolver<grid>::getCell( const point_t& p, bool create )
{
	auto i = cell_map_.find( p );
	if( i != cell_map_.end() ) {
		return cells_[i->second].index_;
	}

	if( create ) {
		cell_index new_index = cells_.size();
		cells_.emplace_back( p, new_index );
		cells_.back().var_ = declareVariable();
		cell_map_[p] = new_index;
		return new_index;
	}

	return -1;
}

template<typename grid>
var_id HeeschSolver<grid>::getCellVariable( const point_t& p )
{
	cell_index index = getCell( p, true );
	return cells_[index].var_;
}

template<typename grid>
var_id HeeschSolver<grid>::getCellVariable( const point_t& p ) const
{
	auto i = cell_map_.find( p );
	if( i != cell_map_.end() ) {
		return cells_[i->second].var_;
	}

	std::cerr << "Tried to look up non-existent cell " << p 
		<< " in clause generation" << std::endl;
	exit( 0 );
	return 0;
}

template<typename grid>
bool HeeschSolver<grid>::hasCell( const point_t& p ) const
{
	return cell_map_.find( p ) != cell_map_.end();
}

template<typename grid>
tile_index HeeschSolver<grid>::createNewTile( const xform_t& T )
{
	tile_index new_index = tiles_.size();
	tiles_.emplace_back( T, new_index );
	tile_map_[T] = new_index;
	tile_info<grid>& ti = tiles_.back();

	for( auto& p : shape_ ) {
		point_t tp = T * p;
		cell_index cidx = getCell( tp, true );
		ti.cells_.push_back( cidx );

		cells_[cidx].tiles_.push_back( new_index );
	}

	return new_index;
}

template<typename grid>
var_id HeeschSolver<grid>::getShapeVariable( const xform_t& T, size_t level )
{
	tile_index index = -1;
	auto i = tile_map_.find( T );

	if( i == tile_map_.end() ) {
		// No tile currently exists at this location, so create one
		// and hook it into the system.
		index = createNewTile( T );
	} else {
		index = i->second;
	}

	// The location is in the map.  Now check if the variable exists
	// for this level.
	tile_info<grid>& ti = tiles_[index];

	auto j = ti.vars_.find( level );
	if( j == ti.vars_.end() ) {
		var_id nv = declareVariable();
		ti.vars_[level] = nv;
		return nv;
	} else {
		return j->second;
	}
}

// Try to retrieve the variable for the given shape at the given level
// if it exists, but don't allocate a new variable if it doesn't.
template<typename grid>
bool HeeschSolver<grid>::getShapeVariable( 
	const xform_t& T, size_t level, var_id& id ) const
{
	auto i = tile_map_.find( T );

	if( i == tile_map_.end() ) {
		return false;
	}

	tile_index index = i->second;

	// The location is in the map.  Now check if the variable exists
	// for this level.
	tile_info<grid>& ti = tiles_[index];

	auto j = ti.vars_.find( level );
	if( j == ti.vars_.end() ) {
		return false;
	}

	id = j->second;
	return true;
}

template<typename grid>
void HeeschSolver<grid>::extendLevelWithTransforms(
	size_t lev, const xform_set<coord_t>& Ts )
{
	size_t sz = tiles_.size();

	for( size_t idx = 0; idx < sz; ++idx ) {
		if( tiles_[idx].hasLevel( lev ) ) {
			xform_t Told = tiles_[idx].T_;

			for( const xform_t& T : Ts ) {
				xform_t Tnew = Told * T;

				// This is a small optimization -- coronas beyond the
				// first can't be anywhere near the kernel.

				// CSK: the check cloud_.isAny( Tnew ) worked for a long
				// time, but was finally broken by a 14-kite in 2023.
				// The problem was that there was a in cell in the halo
				// of a Level-1 shape that wasn't covered by any Level-2
				// shapes, because all possibilities happened to be near
				// the kernel and were therefore eliminated.  Then, when
				// we reached Level 2 and that shape tried to activate its
				// halo, it discovered that some of those cells didn't have
				// SAT variables!  

				// It might be possible to keep this optimization and then
				// to catch this problem at the other end, by treating
				// the failure of getCellVariable() not as a fatal error,
				// but as a sign that no corona can exist.  But it's a 
				// small optimization, so just ignore it for now.
				if( /* cloud_.isAny( Tnew ) || */ Tnew.isIdentity() ) {
					continue;
				}

				getShapeVariable( Tnew, lev + 1 );
			}
		}
	}
}

template<typename grid>
void HeeschSolver<grid>::increaseLevel()
{
	++level_;
	if( level_ == 1 ) {
		for( auto& T : cloud_.adjacent_ ) {
			getShapeVariable( T, 1 );
		}
	} else {
		extendLevelWithTransforms( level_ - 1, cloud_.adjacent_ );
	}
}

template<typename grid>
void HeeschSolver<grid>::addHolesToLevel()
{
	extendLevelWithTransforms( level_ - 1, cloud_.adjacent_hole_ );
}

inline CMSat::Lit pos( var_id id )
{
    return CMSat::Lit( id, false );
}

inline CMSat::Lit neg( var_id id )
{
	return CMSat::Lit( id, true );
}

template<typename grid>
void HeeschSolver<grid>::getClauses(
	CMSat::SATSolver& solv, bool allow_holes ) const
{
	std::vector<CMSat::Lit> cl;

	cl.push_back( pos( tiles_[0].vars_.at( 0 ) ) );
	solv.add_clause( cl );

	// If a copy of S is used, then its cells are used.
	cl.resize( 2 );
	for( auto& ti : tiles_ ) {
		for( auto& p : shape_ ) {
			point_t tp = ti.T_ * p;
			cl[1] = pos( getCellVariable( tp ) );

			for( auto& i : ti.vars_ ) {
				cl[0] = neg( i.second );
				solv.add_clause( cl );
			}
		}
	}

	// If a cell is used, then some copy of S must use it.
	for( auto& ci : cells_ ) {
		cl.clear();
		cl.push_back( neg( ci.var_ ) );
		for( auto& tindex : ci.tiles_ ) {
			auto& ti = tiles_[tindex];
			for( auto& i : ti.vars_ ) {
				cl.push_back( pos( i.second ) );
			}
		}
		solv.add_clause( cl );
	}

//	std::cerr << "Interior copies of S must have covered haloes."
//		<< std::endl;
	// If a copy of S is used in an interior corona (a k-corona for k < n),
	// then that copy’s halo cells must be used.
	cl.resize( 2 );
	for( auto& ti : tiles_ ) {
		for( auto& i : ti.vars_ ) {
			if( i.first < level_ ) {
				// This is a tile variable at an inner corona.
				cl[0] = neg( i.second );
				for( auto& p : cloud_.halo_ ) {
					point_t tp = ti.T_ * p;
					cl[1] = pos( getCellVariable( tp ) );
					solv.add_clause( cl );
				}
			}
		}
	}
//	std::cerr << "Done." << std::endl;

	// Used copies of S cannot overlap.
	cl.resize( 2 );
	for( auto& ti : tiles_ ) {
		for( auto& M : cloud_.overlapping_ ) {
			xform_t Tn = ti.T_ * M;
			// OK, so is there a tile located at Tn?
			tile_index index = getTile( Tn );
			if( index == -1 ) {
				continue;
			}
			auto& tj = tiles_[index];

			// There's a tile here. Prevent all pairwise overlaps
			// at all levels.
			for( auto& i : ti.vars_ ) {
				for( auto& j : tj.vars_ ) {
					cl[0] = neg( i.second );
					cl[1] = neg( j.second );
					solv.add_clause( cl );
				}
			}
		}
	}
	
	// Used copies of S cannot overlap.
	// If a copy of S is used in a k-corona, it must be adjacent to a copy
	// in a (k−1)-corona
	// If a copy of S is used in a k-corona, it cannot be adjacent to a
	// copy in an m-corona for m < k − 1.
	for( auto& ti : tiles_ ) {
		for( auto& i : ti.vars_ ) {
			size_t k = i.first;
			if( k < 1 ) {
				continue;
			}
			cl.clear();
			cl.push_back( neg( i.second ) );
			for( auto& M : cloud_.adjacent_ ) {
				xform_t Tn = ti.T_ * M;
				tile_index index = getTile( Tn );
				if( index == -1 ) {
					continue;
				}
				auto& tj = tiles_[index];
				for( auto& j : tj.vars_ ) {
					if( j.first == k - 1 ) {
						cl.push_back( pos( j.second ) );
					} else if( j.first < k - 1 ) {
						std::vector<CMSat::Lit> cl2;
						cl2.push_back( neg( i.second ) );
						cl2.push_back( neg( j.second ) );
						solv.add_clause( cl2 );
					}
				}
			}

			if( cl.size() > 1 ) {
				solv.add_clause( cl );
			}

			if( !allow_holes && (k == level_) ) {
				// outermost corona, no holes allowed.  Walk over the
				// hole_adjacencies and forbid them.
				cl.resize( 2 );

				for( auto& M : cloud_.adjacent_hole_ ) {
					xform_t Tn = ti.T_ * M;
					tile_index index = getTile( Tn );
					if( index == -1 ) {
						continue;
					}
					auto& tj = tiles_[index];
					if( tj.hasLevel( k ) ) {
						cl[0] = neg( i.second );
						cl[1] = neg( tj.vars_.at( k ) );
						solv.add_clause( cl );
					}
				}
			}
		}
	}
}

template<typename grid>
void HeeschSolver<grid>::getSolution( 
	const CMSat::SATSolver& solv, Solution<coord_t>& ret ) const
{
	ret.clear();
	const std::vector<CMSat::lbool>& model = solv.get_model();
	for( auto& ti : tiles_ ) {
		for( auto& i : ti.vars_ ) {
			if( model[i.second] == CMSat::l_True ) {
				ret.emplace_back( i.first, ti.T_ );
				break;
			}
		}
	}
}

template<typename grid> bool HeeschSolver<grid>::isIsohedral(
	bool get_solution, Solution<coord_t>& ret ) 
{
	if( !cloud_.surroundable_ ) {
		return false;
	}

	return true;

#if 0
	// Everything in here was just some preliminary playing around and
	// doesn't work at all.

	// Implicitly var_id 1
	xform_t I = xform_t {};

	if( level_ == 0 ) {
		increaseLevel();
	}

	// Find all tile placements whose inverses are also valid
	// placements
	xform_map<coord_t,var_id> eligible;
	for( const auto& T : cloud_.adjacent_ ) {
		xform Ti = T.invert();
		if( cloud_.isAdjacent( Ti ) ) {
			eligible[T] = 0;
		}
	}

	CMSat::SATSolver solv;
	solv.new_vars( eligible.size() + 1 );

	// Assign variables.
	var_id id = 1;
	for( auto& i : eligible ) {
		i.second = id;
		++id;
	}

	std::vector<CMSat::Lit> cl;

	// Every halo cell is used
	for( const auto& p : cloud_.halo_ ) {
		cl.clear();
		const auto& ci = cells_[ cell_map_[p] ];
		for( const auto tindex : ci.tiles_ ) {
			auto i = eligible.find( tiles_[tindex].T_ );
			if( i != eligible.end() ) {
				cl.push_back( pos( i->second ) );
			}
		}
		solv.add_clause( cl );
	}

	cl.resize( 2 );

	// No two tiles overlap
	size_t idx = 0;
	for( auto i : eligible ) {
		size_t jdx = 0;
		for( auto j : eligible ) {
			++jdx;
			if( jdx == idx ) {
				break;
			}
			if( cloud_.isOverlap( i.first * j.first.invert() ) ) {
				cl[0] = neg( i.second );
				cl[1] = neg( j.second );
				solv.add_clause( cl );
			}
		}
		++idx;
	}


	/*
	// For every halo cell around the central tile, exactly one
	// eligible tile covers it.
	for( const auto& p : cloud_.halo_ ) {
		const auto& ci = cells_[ cell_map_[p] ];
		for( const auto tindex : ci.tiles_ ) {
			auto i = eligible.find( tiles_[tindex].T_ );
			if( i != eligible.end() ) {
				ids.push_back( i->second );
			}
		}
		solv.add_xor_clause( ids, true );
	}
	*/
	

	// Every eligible transform implies its inverse -- they must be
	// used in pairs.
	for( auto i : eligible ) {
		auto j = eligible.find( i.first.invert() );
		cl[0] = neg( i.second );
		cl[1] = pos( j->second );
		solv.add_clause( cl );
	}

	cl.resize( 1 );
	cl[0] = pos( 0 );
	solv.add_clause( cl );

	if( solv.solve() == CMSat::l_True ) {
		if( get_solution ) {
			ret.clear();
			const std::vector<CMSat::lbool>& model = solv.get_model();
			ret.emplace_back( 0, I );
			for( auto i : eligible ) {
				if( model[i.second] == CMSat::l_True ) {
					ret.emplace_back( 1, i.first );
				}
			}
		}
		return true;
	} else {
		return false;
	}
#endif
}

template<typename grid>
bool HeeschSolver<grid>::hasCorona( 
	bool get_solution, bool& has_holes, Solution<coord_t>& soln ) 
{
	if( level_ == 0 ) {
		// A hole-free 0-corona always exists.
		has_holes = false;
		if( get_solution ) {
			soln.push_back( std::make_pair( 0, grid::orientations[0] ) );
		}
		// std::cout << "Asking for 0-corona" << std::endl;
		return true;
	}

	if( !cloud_.surroundable_ ) {
		// std::cout << "Not surroundable at all" << std::endl;
		return false;
	}

	CMSat::SATSolver solver;
	solver.new_vars( next_var_ );

	getClauses( solver, false );
	if( solver.solve() == CMSat::l_True ) {
		// Got a solution, but it may have large holes.  Need to find
		// them and iterate until they're gone.

		has_holes = true;
		if( get_solution ) {
			getSolution( solver, soln );
			// debugSolution( std::cout, shape_, soln );
		}

		while( true ) {
			const std::vector<CMSat::lbool>& model = solver.get_model();
			HoleFinder<grid> finder { shape_ };

			for( auto& ti : tiles_ ) {
				for( auto i : ti.vars_ ) {
					if( model[i.second] == CMSat::l_True ) {
						finder.addCopy( ti.index_, ti.T_ );
						break;
					}
				}
			}

			std::vector<std::vector<tile_index>> holes;
			if( !finder.getHoles( holes ) ) {
				// Found a hole-free solution!
				has_holes = false;
				if( get_solution ) {
					getSolution( solver, soln );
				}
				return true;
			}

			std::vector<CMSat::Lit> cl;
			for( auto& hole : holes ) {
				cl.clear();
				// std::cout << "Forbidding a hole:";
				for( auto& index : hole ) {
					// We know that there's a variable at the top level,
					// otherwise we wouldn't have found a hole in the
					// first place.
					cl.push_back( neg( tiles_[index].vars_.at( level_ ) ) );
					// std::cout << " " << index;
				}
				// std::cout << std::endl;
				solver.add_clause( cl );
			}

			if( solver.solve() == CMSat::l_False ) {
				// Ran out of options; revert to the already captured 
				// solution with holes.
				// std::cout << "No longer solvable" << std::endl;
				return true;
			}
		}
	} else {
		// No solution found yet.  Try a larger solution by allowing
		// adjacency holes in the top corona.
		// std::cout << "Adding holes to level" << std::endl;
		addHolesToLevel();

		CMSat::SATSolver solver;
		solver.new_vars( next_var_ );
		getClauses( solver, true );
		if( solver.solve() == CMSat::l_True ) {
			has_holes = true;
			if( get_solution ) {
				getSolution( solver, soln );
			}
			// std::cout << "Found solution with holes" << std::endl;
			return true;
		} else {
			// std::cout << "No solution with holes" << std::endl;
			return false;
		}
	}
}

template<typename grid>
void HeeschSolver<grid>::allCoronas( std::vector<Solution<coord_t>>& solns ) 
{
	if( !cloud_.surroundable_ ) {
		// std::cout << "Not surroundable at all" << std::endl;
		return;
	}

	solns.clear();

	CMSat::SATSolver solver;
	solver.new_vars( next_var_ );

	getClauses( solver, false );
	while( solver.solve() == CMSat::l_True ) {
		// Got a solution, but it may have large holes.  Need to find
		// them and iterate until they're gone.
		// std::cout << "found a solution" << std::endl;

		HoleFinder<grid> finder { shape_ };

		std::vector<CMSat::Lit> cl;
		const std::vector<CMSat::lbool>& model = solver.get_model();

		// Get tile info for hole detection, while simultaneously
		// building clause for forbidding this solution.
		for( auto& ti : tiles_ ) {
			for( auto& i : ti.vars_ ) {
				if( model[i.second] == CMSat::l_True ) {
					finder.addCopy( ti.index_, ti.T_ );
					cl.push_back( neg( i.second ) );
				}
			}
		}

		std::vector<std::vector<tile_index>> holes;
		if( !finder.getHoles( holes ) ) {
			// No holes, so keep the solution
			Solution<coord_t> soln;
			getSolution( solver, soln );
			solns.push_back( soln );
		}

		// Suppress this solution and keep going.
		solver.add_clause( cl );
	}
}

template<typename grid>
void HeeschSolver<grid>::debug( std::ostream& os ) const
{
	for( auto& ci : cells_ ) {
		os << "  Cell #" << ci.index_ << " at " << ci.pos_ 
		   << ", var = " << ci.var_ << std::endl;
		os << "    Tiles:";
		for( auto& index: ci.tiles_ ) {
			os << " " << index;
		}
		os << std::endl;
	}
	for( auto& ti : tiles_ ) {
		os << "  Tile #" << ti.index_ << " at " << ti.T_ << ":" << std::endl;
		os << "    Cells:";
		for( auto& i : ti.cells_ ) {
			os << " " << i << ":" << cells_[i].pos_;
		}
		os << std::endl;
		os << "    Halo:";
		for( auto& p : cloud_.halo_ ) {
			auto hp = ti.T_ * p;
			os << " " << hp ;
		}	
		os << std::endl;
		os << "    Vars:";
		for( auto i : ti.vars_ ) {
			os << " [" << i.first << "," << i.second << "]";
		}
		os << std::endl;
	}
}
