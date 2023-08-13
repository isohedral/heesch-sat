#pragma once

#include <set>
#include <vector>

#include "grid.h"

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
		// size_t evens = 0;

		for( auto& p : shape ) {
			if( p.second == OCCUPIED ) {
				pts.push_back( p.first );
				/*
				if( (p.first.x_ + p.first.y_) % 2 == 0 ) {
					++evens;
				}
				*/
			}
		}

		// if( evens == 3 ) {
			out( origin, pts );
		// }

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

	std::cerr << "  ... New sym shape, saving it." << std::endl;
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

/*
	// Not this way.  Need to find the lex-first point in the shape, and
	// move that to whatever origin is appropriate for its type.
	for( auto& p : nshape ) {
		if( grid::translatable( p, origin ) ) {
			mpt = p;
			break;
		}
	}

	point_t d = origin - mpt;
	*/

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
