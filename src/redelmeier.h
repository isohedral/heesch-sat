#pragma once

#include <set>
#include <vector>

#include "grid.h"
#include "shape.h"

template<typename grid>
using polyform_cb = std::function<
	void( const typename grid::point_t&, 
		  const std::vector<typename grid::point_t>& )>;

// Just enumerate fixed polyforms over this grid.  It's important to
// enumerate exhaustively; otherwise, it's possible that FreeFilter will
// get confused about canonicity and miss some free polyforms.
template<typename grid>
class RedelmeierSimple
{
    using coord_t = typename grid::coord_t;
    using point_t = typename grid::point_t;
    using shape_t = std::vector<point_t>;

    enum CellStatus {
        FREE, OCCUPIED, UNTRIED, BLOCKED, REACHABLE
    };

	using cell_map = point_map<coord_t, CellStatus>;

	cell_map shape;
	point_t origin;
	std::vector<point_t> untried;

public:
	explicit RedelmeierSimple()
		: shape {}
		, origin {}
		, untried {}
	{}

	template<typename CB = polyform_cb<grid>>
	size_t solve( size_t size, CB out )
	{
		cell_map shape;
		size_t total = 0;

		for( const auto& o : grid::origins ) {
			origin = o;
			shape.clear();
			untried.clear();
			untried.push_back( o );
			total += solve( size, 0, out );
		}

		return total;
	}

private:
	bool contains( const point_t& p )
	{
		return shape.find( p ) != shape.end();
	}

	template<typename CB>
	size_t solve( size_t size, size_t from, CB out );
};

template<typename grid>
using adj_info = std::pair<size_t, xform<typename grid::coord_t>>;

template<typename grid>
struct adj_hash
{
	size_t operator()( const adj_info<grid>& adj ) const
	{
		size_t res = adj.first;
		boost::hash_combine( res, adj.second.hash() );
		return res;
	}
};

// Enuemrate fixed polyforms whose "cells" are themselves polyforms
// in the underlying grid.
template<typename grid>
class RedelmeierCompound
{
    using coord_t = typename grid::coord_t;
    using point_t = typename grid::point_t;
    using xform_t = typename grid::xform_t;
    using adj_t = adj_info<grid>;

    enum CellStatus {
        FREE, OCCUPIED, UNTRIED, BLOCKED, REACHABLE
    };

	using cell_map = std::unordered_map<adj_t, CellStatus, adj_hash<grid>>;

	std::vector<Shape<grid>> shapes;
	// For each shape, a vector of its possible adjacencies
	std::vector<std::vector<adj_t>> adjs;

	cell_map shape;
	std::vector<adj_t> untried;

public:
	explicit RedelmeierCompound( const std::vector<Shape<grid>>& shapes )
		: shapes { shapes }
		, adjs { shapes.size() }
		, shape {}
		, untried {}
	{
		calculateAdjacencies();

/*
		std::cerr << "Adjacencies" << std::endl;
		for( size_t idx = 0; idx < adjs.size(); ++idx ) {
			std::cerr << "=== " << idx << " ===" << std::endl;
			for( const auto& adj : adjs[idx] ) {
				for( const auto& pp : shapes[idx] ) {
					std::cerr << pp.x_ << ' ' << pp.y_ << ' ';
				}
				for( const auto& pp : shapes[adj.first] ) {
					point_t q = adj.second * pp;
					std::cerr << q.x_ << ' ' << q.y_ << ' ';
				}
				std::cerr << std::endl;
				std::cerr << "Hc = 0 Hh = 0" << std::endl;
				std::cerr << "1" << std::endl;
				std::cerr << "0 ; <1,0,0,0,1,0>" << std::endl;
			//	std::cerr << "1 ; " << adj.second << std::endl;
			}
		}
	*/
	}

	template<typename CB = polyform_cb<grid>>
	size_t solve( size_t size, CB out )
	{
		cell_map shape;
		size_t total = 0;

		for( size_t idx = 0; idx < shapes.size(); ++idx ) {
			shape.clear();
			untried.clear();
			untried.emplace_back( idx, xform_t {} );
			total += solve( size, 0, out );
		}

		return total;
	}

private:
	void calculateAdjacencies();

	/*
	void set( const Shape<grid>& sh, const xform_t& T, CellStatus cs )
	{
		if( cs == FREE ) {
			for( const auto& p : sh ) {
				shape.erase( T * p );
			}
		} else {
			for( const auto& p : sh ) {
				shape[ T * p ] = cs;
			}
		}
	}

	bool contains( const Shape<grid>& sh, const xform_t& T )
	{
		for( const auto& p : sh ) {
			if( shape.find( T * p ) != shape.end() ) {
				return true;
			}
		}
		return false;
	}
	*/

	bool contains( const adj_t& adj )
	{
		return shape.find( adj ) != shape.end();
	}

	template<typename CB>
	size_t solve( size_t size, size_t from, CB out );
};

// Filter fixed polyforms to obtain free polyforms
template<typename grid>
class FreeFilter
{
    using coord_t = typename grid::coord_t;
    using point_t = typename grid::point_t;
    using xform_t = typename grid::xform_t;
    using shape_t = std::vector<point_t>;

	RedelmeierSimple<grid> fixed;
	std::vector<shape_t> syms;
	bool debug;

public:
	explicit FreeFilter()
		: fixed {}
		, syms {}
		, debug { false }
	{}

	template<typename CB = polyform_cb<grid>>
	size_t solve( size_t size, CB out )
	{
		return fixed.solve( size, [this, out]( 
			const point_t& origin, const std::vector<point_t>& shape ) {
				if( checkShape( origin, shape ) ) {
					out( origin, shape ); 
				}
			} );
	}

	void setDebug( bool b )
	{
		debug = b;
	}

private:
	bool checkShape( const point_t& origin, const shape_t& shape );
	static int compareShapes( const shape_t& A, const shape_t& B );
	static shape_t transformShape( 
		const shape_t& shape, const xform_t& T, const point_t& origin );

	static void dbg( const std::string& s, const shape_t& shape )
	{
		std::cerr << s;

		for( const auto& p : shape ) {
			std::cerr << ' ' << p.x_ << ' ' << p.y_;
		}

		std::cerr << std::endl;
	}
};

template<typename grid>
template<typename CB>
size_t RedelmeierSimple<grid>::solve( size_t size, size_t from, CB out )
{
	if( size == 0 ) {
		std::vector<point_t> pts;

		for( auto& p : shape ) {
			if( p.second == OCCUPIED ) {
				pts.push_back( p.first );
			}
		}

		out( origin, pts );
		return 1;
	} else {
		size_t total = 0;
		size_t usz = untried.size();

		for( size_t idx = from; idx < usz; ++idx ) {
			point_t p = untried[idx];
			shape[p] = OCCUPIED;

			for( auto pn : edge_neighbours<grid> { p } ) {
				if( !(pn < origin) && !contains( pn ) ) {
					// FIXME -- this is an inefficient way to check
					// whether this is a new untried cell.
					if( std::find( untried.begin(), untried.end(), pn )
							== untried.end() ) {
						untried.push_back( pn );
					}
				}
			}

			total += solve( size - 1, idx + 1, out );
			shape[p] = REACHABLE;
			untried.resize( usz );
		}

		for( size_t idx = from; idx < usz; ++idx ) {
			shape.erase( untried[idx] );
		}

		return total;
	}
}

template<typename grid>
bool FreeFilter<grid>::checkShape( const point_t& origin, const shape_t& shape )
{
	bool is_symmetric = false;

	shape_t cshape = transformShape( shape, xform_t {}, origin );
	if( debug ) {
		dbg( "Incoming ", shape );
		std::cerr << "Origin: " << origin << std::endl;
		dbg( "Checking ", cshape );
	}

	// If the shape is asymmetric, output it if it's the lexicographically
	// first among its transformed copies.  If a symmetry is detected, 
	// deal with that separately.

	for( size_t idx = 1; idx < grid::num_orientations; ++idx ) {
		const xform_t& T = grid::orientations[idx];
		shape_t tshape = transformShape( cshape, T, origin );

		int cmp = compareShapes( tshape, cshape );
		if( cmp < 0 ) {
			// Sorry, you're not canonical
			if( debug ) {
				dbg( "  ... Not canonical: ", tshape );
			}
			return false;
		} else if( cmp == 0 ) {
			// Ooh, you're symmetric
			if( debug ) {
				std::cerr << "  ... Symmetric!" << std::endl;
			}
			is_symmetric = true;
			break;
		}
	}

	if( !is_symmetric ) {
		// No symmetries, so you must be canonical
		return true;
	}

	// Store symmetric shapes explicitly.  Must store the lex-min transformed
	// copy.  Repeat the transform process above -- costs a bit, but it
	// happens rarely.
	shape_t min_shape = cshape;
	for( size_t idx = 1; idx < grid::num_orientations; ++idx ) {
		const xform_t& T = grid::orientations[idx];
		shape_t tshape = transformShape( cshape, T, origin );
		int cmp = compareShapes( tshape, min_shape );
		if( cmp < 0 ) {
			min_shape = tshape;
		}
	}

	if( debug ) {
		dbg( "  ... Converted to ", min_shape );
	}

	for( const auto& sshape : syms ) {
		if( compareShapes( sshape, min_shape ) == 0 ) {
			// We've already seen this one
			if( debug ) {
				std::cerr << "  ... Already seen this shape" << std::endl;
			}
			return false;
		}
	}

	if( debug ) {
		std::cerr << "  ... New sym shape, saving it." << std::endl;
	}
	syms.push_back( min_shape );
	return true;
}

template<typename grid>
int FreeFilter<grid>::compareShapes( const shape_t& A, const shape_t& B )
{
	// Lex-compare the points in two shapes, which are presumed to contain
	// the same numbers of points.

	for( size_t idx = 0; idx < A.size(); ++idx ) {
		if( A[idx] < B[idx] ) {
			return -1;
		} else if( B[idx] < A[idx] ) {
			return 1;
		}
	}

	return 0;
}

template<typename grid>
typename FreeFilter<grid>::shape_t FreeFilter<grid>::transformShape( 
	const shape_t& shape, const xform_t& T, const point_t& origin )
{
	// Apply the transform matrix
	shape_t nshape;
	for( auto& p : shape ) {
		nshape.push_back( T * p );
	}

	// Translate so that the lex-first-est point lies at the origin.
	std::sort( nshape.begin(), nshape.end() );

	point_t mpt;
	point_t d;

	// FIXME This doesn't have to be a loop, it can be sped up using
	// a method like Ava's "getTileType", which selects an appropriate origin
	// directly from nshape[0]'s x and y coordinates.
	for( auto& o : grid::origins ) {
		if( grid::translatable( o, nshape[0] ) ) {
			d = o - nshape[0];
			break;
		}
	}

	for( auto& p : nshape ) {
		p = p + d;
	}

	return nshape;
}

template<typename grid>
void RedelmeierCompound<grid>::calculateAdjacencies()
{
	// This basically must recapitulate all the work done in the Cloud
	// class to calculate adjacencies, but it has to deal with pairwise
	// edge adjacencies between distinct units.  Oy.

	// All possible orientations of all shapes, with their transformed
	// halos.
	struct ShapeInfo
	{
		size_t idx;
		xform_t T;

		Shape<grid> shape;
		Shape<grid> halo;

		explicit ShapeInfo( size_t idx, const xform_t& T, 
				const Shape<grid>& shape, const Shape<grid>& halo )
			: idx { idx }
			, T { T }
			, shape { shape }
			, halo { halo }
		{}
	};

	std::vector<ShapeInfo> all_shapes;

	// Start by populating the array with untransformed copies of the units.
	for( size_t idx = 0; idx < shapes.size(); ++idx ) {
		const Shape<grid>& sh = shapes[idx];
		Shape<grid> halo;

		sh.getEdgeHalo( halo );
		all_shapes.emplace_back( idx, grid::orientations[0], sh, halo );
	}

	// Generate the remaining infos by applying all non-identity 
	// orientations to all input shapes.
	Shape<grid> oshape;
	Shape<grid> ohalo;

	for( size_t idx = 0; idx < shapes.size(); ++idx ) {
		for( size_t tidx = 1; tidx < grid::num_orientations; ++tidx ) {
			// Need to get this reference afresh every time, because
			// the emplace_back call below can cause the reference to
			// become invalid in the outer loop.
			const ShapeInfo& si = all_shapes[idx];

			const xform_t& T = grid::orientations[tidx];

			oshape.reset( si.shape, T );
			ohalo.reset( si.halo, T );
			all_shapes.emplace_back( idx, T, oshape, ohalo );
		}
	}

	Shape<grid> new_shape;

	// Check all pairwise combinations of these puppies.
	// all_shapes doesn't change any longer, so references are fine.
	for( size_t idx = 0; idx < shapes.size(); ++idx ) {
		const Shape<grid>& sh = all_shapes[idx].shape;
		const Shape<grid>& halo = all_shapes[idx].halo;

		for( const auto& oa : all_shapes ) {
			const Shape<grid> osh = oa.shape;
			const xform_t& T = oa.T;
			size_t oidx = oa.idx;
			// Check all translations of osh into the halo of oa

			for( const point_t& hp : halo ) {
				for( const point_t& osp : osh ) {
					if( !grid::translatable( osp, hp ) ) {
						// Easy: not a legal translation.
						continue;
					}

					xform_t Tnew = xform_t {}.translate( hp - osp );
					// Translate osh so it occupies the halo cell
					new_shape.reset( osh, Tnew );

					// If new_shape overlaps sh, stop immediately
					if( sh.intersects( new_shape ) ) {
						continue;
					}

					// Construct a compound of both shapes and make sure
					// they don't enclose an internal hole
					new_shape.add( sh );
					if( !new_shape.simplyConnected() ) {
						continue;
					}

					// You've passed all the tests, congratulations!  You're
					// a legitimate neighbour for this unit.
					Tnew = T.translate( hp - osp );
					adjs[idx].emplace_back( oidx, Tnew );
				}
			}
		}
	}
}

template<typename grid>
template<typename CB>
size_t RedelmeierCompound<grid>::solve( size_t size, size_t from, CB out )
{
	if( size == 0 ) {
		std::vector<point_t> pts;
		point_set<typename grid::coord_t> pset;

		for( auto& a : shape ) {
			if( a.second == OCCUPIED ) {
				const adj_t& adj = a.first;
				for( const auto& p : shapes[adj.first] ) {
					point_t np = adj.second * p;
					if( pset.find( np ) != pset.end() ) {
						return 0;
					}
					pset.insert( np );
					pts.push_back( np );
				}
			}
		}

		std::sort( pts.begin(), pts.end() );
		point_t o;
		for( const auto& p : grid::origins ) {
			if( grid::translatable( p, pts[0] ) ) {
				o = p;
				break;
			}
		}
		point_t v = o - pts[0];
		for( auto& p : pts ) {
			p = p + v;
		}

		Shape<grid> shp;
		for( const auto& p : pts ) {
			shp.add( p );
		}
		shp.complete();
		if( !shp.simplyConnected() ) {
			std::cerr << "Made something not simply connected " << 
				v << std::endl;
		}

		out( point_t {}, pts );
		return 1;
	} else {
		size_t total = 0;
		size_t usz = untried.size();

		for( size_t idx = from; idx < usz; ++idx ) {
			const adj_t utadj = untried[idx];
			size_t sidx = untried[idx].first;

			shape[utadj] = OCCUPIED;

			for( const auto& oa : adjs[sidx] ) {
				xform_t Tnew = utadj.second * oa.second;
				adj_t nadj { oa.first, Tnew };

				if( !contains( nadj ) ) {
					if( std::find( untried.begin(), untried.end(), nadj )
							== untried.end() ) {
						untried.push_back( nadj );
					}
				}
			}

			total += solve( size - 1, idx + 1, out );
			shape[utadj] = REACHABLE;
			untried.resize( usz );
		}

		for( size_t idx = from; idx < usz; ++idx ) {
			shape[untried[idx]] = FREE;
		}

		return total;
	}
}
