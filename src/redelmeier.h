#pragma once

#include <set>
#include <vector>

#include "grid.h"
#include "shape.h"

template<typename grid>
using polyform_cb = std::function<void(const Shape<grid>&)>;

// Just enumerate fixed polyforms over this grid.  It's important to
// enumerate exhaustively; otherwise, it's possible that FreeFilter will
// get confused about canonicity and miss some free polyforms.
template<typename grid>
class RedelmeierSimple
{
    using coord_t = typename grid::coord_t;
    using point_t = typename grid::point_t;
    using shape_t = Shape<grid>;

    enum CellStatus {
        FREE, OCCUPIED, UNTRIED, BLOCKED, REACHABLE
    };

	using cell_map = point_map<coord_t, CellStatus>;

	cell_map cellmap;
	point_t origin;
	std::vector<point_t> untried;

public:
	explicit RedelmeierSimple()
		: cellmap {}
		, origin {}
		, untried {}
	{}

	size_t solve( size_t size, polyform_cb<grid> out )
	{
		size_t total = 0;

		for( const auto& o : grid::origins ) {
			origin = o;
			cellmap.clear();
			untried.clear();
			untried.push_back( o );
			total += solve( size, 0, out );
		}

		return total;
	}

private:
	bool contains( const point_t& p )
	{
		return cellmap.find( p ) != cellmap.end();
	}

	size_t solve( size_t size, size_t from, const polyform_cb<grid>& out );
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
	using shape_t = Shape<grid>;
    using adj_t = adj_info<grid>;

    enum CellStatus {
        FREE, OCCUPIED, UNTRIED, BLOCKED, REACHABLE
    };

	using cell_map = std::unordered_map<adj_t, CellStatus, adj_hash<grid>>;

	std::vector<shape_t> shapes;
	// For each shape, a vector of its possible adjacencies
	std::vector<std::vector<adj_t>> adjs;

	cell_map cellmap;
	std::vector<adj_t> untried;

public:
	explicit RedelmeierCompound( 
			const std::vector<shape_t>& shapes )
		: shapes { shapes }
		, adjs { shapes.size() }
		, cellmap {}
		, untried {}
	{
		calculateAdjacencies();

/*
		for( size_t idx = 0; idx < adjs.size(); ++idx ) {
			std::cerr << "Shape " << idx << " has " << adjs[idx].size()
				<< " adjacencies" << std::endl;
		}

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

	size_t solve( size_t size, polyform_cb<grid> out )
	{
		size_t total = 0;

		for( size_t idx = 0; idx < shapes.size(); ++idx ) {
			cellmap.clear();
			untried.clear();
			untried.emplace_back( idx, xform_t {} );
			total += solve( size, 0, out );
		}

		return total;
	}

	static void calculateInequivalentOrientations(
		const shape_t& shp, std::vector<xform_t>& Ts );

private:
	void calculateAdjacencies();

	bool contains( const adj_t& adj )
	{
		return cellmap.find( adj ) != cellmap.end();
	}

	size_t solve( size_t size, size_t from, const polyform_cb<grid>& out );

};

// Filter fixed polyforms to obtain free polyforms
template<typename grid>
class FreeFilter
{
    using coord_t = typename grid::coord_t;
    using point_t = typename grid::point_t;
    using xform_t = typename grid::xform_t;
    using shape_t = Shape<grid>;

	std::vector<shape_t> syms;
	bool debug;

public:
	explicit FreeFilter()
		: syms {}
		, debug { false }
	{}

	template<class Sub>
	size_t solve( size_t size, Sub& sub, polyform_cb<grid> out )
	{
		return sub.solve( size, [this, out]( const shape_t& shape ) {
			if( checkShape( shape ) ) {
				out( shape ); 
			}
		} );
	}

	void setDebug( bool b )
	{
		debug = b;
	}

private:
	bool checkShape( const shape_t& shape );

	static void dbg( const std::string& s, const shape_t& shape )
	{
		std::cerr << s;

		for( const auto& p : shape ) {
			std::cerr << ' ' << p.x_ << ' ' << p.y_;
		}

		std::cerr << std::endl;
	}
};

// A heavyweight tool for filtering out duplicates.
template<typename grid>
class CanonSortUniq
{
    using coord_t = typename grid::coord_t;
    using point_t = typename grid::point_t;
    using xform_t = typename grid::xform_t;
    using shape_t = Shape<grid>;

	std::vector<shape_t> all;

public:
	explicit CanonSortUniq()
	{}

	template<class Sub>
	size_t solve( size_t size, Sub& sub, polyform_cb<grid> out )
	{
		sub.solve( size, [this]( const shape_t& shape ) {
			// FIXME
			// We could do a "are you the canonical shape" check, if we
			// were certain that the enumeration algorithm actually 
			// generates all orientations of a polyform.
			all.push_back( canonicalize( shape ) );
		} );
		
		std::sort( all.begin(), all.end(), []( const auto& a, const auto& b ) {
			return a.compare( b ) < 0;
		} );

		size_t count = 1;
		out( all[0] );
	
		for( size_t idx = 1; idx < all.size(); ++idx ) {
			if( all[idx-1].compare( all[idx] ) != 0 ) {
				++count;
				out( all[idx] );
			}
		}

		return count;
	}

private:
	static shape_t canonicalize( const shape_t& shp );
};

template<typename grid>
size_t RedelmeierSimple<grid>::solve( 
	size_t size, size_t from, const polyform_cb<grid>& out )
{
	if( size == 0 ) {
		Shape<grid> shp;

		for( auto& p : cellmap ) {
			if( p.second == OCCUPIED ) {
				shp.add( p.first );
			}
		}

		shp.complete();
		out( shp );
		return 1;
	} else {
		size_t total = 0;
		size_t usz = untried.size();

		for( size_t idx = from; idx < usz; ++idx ) {
			point_t p = untried[idx];
			cellmap[p] = OCCUPIED;

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
			cellmap[p] = REACHABLE;
			untried.resize( usz );
		}

		for( size_t idx = from; idx < usz; ++idx ) {
			cellmap.erase( untried[idx] );
		}

		return total;
	}
}

template<typename grid>
bool FreeFilter<grid>::checkShape( const shape_t& shape )
{
	bool is_symmetric = false;

	shape_t cshape { shape };
	shape_t tshape { shape };
	cshape.untranslate();

	if( debug ) {
		dbg( "Incoming ", shape );
		dbg( "Checking ", cshape );
	}

	// If the shape is asymmetric, output it if it's the lexicographically
	// first among its transformed copies.  If a symmetry is detected, 
	// deal with that separately.

	for( size_t idx = 1; idx < grid::num_orientations; ++idx ) {
		const xform_t& T = grid::orientations[idx];
		tshape.reset( cshape, T );
		tshape.untranslate();

		int cmp = tshape.compare( cshape );

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
	shape_t min_shape { cshape };
	for( size_t idx = 1; idx < grid::num_orientations; ++idx ) {
		const xform_t& T = grid::orientations[idx];
		tshape.reset( cshape, T );
		tshape.untranslate();

		int cmp = tshape.compare( min_shape );
		if( cmp < 0 ) {
			min_shape = tshape;
		}
	}

	if( debug ) {
		dbg( "  ... Converted to ", min_shape );
	}

	for( const auto& sshape : syms ) {
		if( sshape.compare( min_shape ) == 0 ) {
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
Shape<grid> CanonSortUniq<grid>::canonicalize( const shape_t& shp )
{
	shape_t canon;
	shape_t tshape;
	bool at_start = true;

	for( const auto& T : grid::orientations ) {
		tshape.reset( shp, T );
		tshape.untranslate();

		if( at_start ) {
			at_start = false;
			canon = tshape;
		} else if( tshape.compare( canon ) < 0 ) {
			canon = tshape;
		}
	}

	return canon;
}

template<typename grid>
void RedelmeierCompound<grid>::calculateInequivalentOrientations(
	const shape_t& shp, std::vector<xform_t>& Ts )
{
	std::vector<shape_t> canon;
	shape_t new_shape;

	Ts.clear();

	for( const auto& T : grid::orientations ) {
		new_shape.reset( shp, T );
		bool ok = true;

		for( const auto& sh : canon ) {
			if( new_shape.equivalent( sh ) ) {
				ok = false;
				break;
			}
		}

		if( ok ) {
			Ts.push_back( T );
			canon.push_back( new_shape );
		}
	}

/*
	std::cerr << "Found " << canon.size() << " inequivalent orientations"
		<< std::endl;
	*/
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

		shape_t shape;
		shape_t halo;

		explicit ShapeInfo( size_t idx, const xform_t& T, 
				const shape_t& shape, const shape_t& halo )
			: idx { idx }
			, T { T }
			, shape { shape }
			, halo { halo }
		{}
	};

	std::vector<ShapeInfo> all_shapes;

	// Start by populating the array with untransformed copies of the units.
	for( size_t idx = 0; idx < shapes.size(); ++idx ) {
		const shape_t& sh = shapes[idx];
		shape_t halo;

		sh.getEdgeHalo( halo );
		all_shapes.emplace_back( 
			idx, grid::orientations[0], sh, std::move( halo ) );
	}

	// Generate the remaining infos by applying all non-identity 
	// orientations to all input shapes.
	shape_t oshape;
	shape_t ohalo;

	for( size_t idx = 0; idx < shapes.size(); ++idx ) {
		std::vector<xform_t> orientations;
		calculateInequivalentOrientations( shapes[idx], orientations );

		for( size_t tidx = 1; tidx < orientations.size(); ++tidx ) {
			const ShapeInfo& si = all_shapes[idx];
			const xform_t& T = orientations[tidx];

			oshape.reset( si.shape, T );
			ohalo.reset( si.halo, T );
			all_shapes.emplace_back( 
				idx, T, std::move( oshape ), std::move( ohalo ) );
		}
	}

	shape_t new_shape;

	// Check all pairwise combinations of these puppies.
	// all_shapes doesn't change any longer, so references are fine.
	for( size_t idx = 0; idx < shapes.size(); ++idx ) {
		const shape_t& sh = all_shapes[idx].shape;
		const shape_t& halo = all_shapes[idx].halo;

		for( const auto& oa : all_shapes ) {
			const shape_t osh = oa.shape;
			const xform_t& T = oa.T;
			size_t oidx = oa.idx;
			// Check all translations of osh into the halo of oa

			point_set<coord_t> t_seen;

			for( const point_t& hp : halo ) {
				for( const point_t& osp : osh ) {
					if( !grid::translatable( osp, hp ) ) {
						// Easy: not a legal translation.
						continue;
					}

					point_t dp = hp - osp;
					if( t_seen.find( dp ) != t_seen.end() ) {
						// We've already tried this translation, no need
						// to try it again.
						continue;
					}

					t_seen.insert( dp );
					// Translate osh so it occupies the halo cell
					new_shape.reset( osh, dp );

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
					xform_t Tnew = T.translate( dp );
					adjs[idx].emplace_back( oidx, Tnew );
				}
			}
		}
	}
}

template<typename grid>
size_t RedelmeierCompound<grid>::solve( 
	size_t size, size_t from, const polyform_cb<grid>& out )
{
	if( size == 0 ) {
		// std::vector<point_t> pts;
		shape_t res;
		point_set<coord_t> pset;

		for( auto& a : cellmap ) {
			if( a.second == OCCUPIED ) {
				const adj_t& adj = a.first;
				for( const auto& p : shapes[adj.first] ) {
					point_t np = adj.second * p;
					if( pset.find( np ) != pset.end() ) {
						return 0;
					}
					pset.insert( np );
					res.add( np );
				}
			}
		}


		// shape_t canon = canonicalize( res );

/*
		std::cerr << "Canonicalizing: ";
		res.debug();
		std::cerr << "        Result: ";
		canon.debug();
		if( res.simplyConnected() != canon.simplyConnected() ) {
			std::cerr << "They disagree!" << std::endl;
		}
	*/

		// out( canon );
		out( res );
		return 1;
	} else {
		size_t total = 0;
		size_t usz = untried.size();

		for( size_t idx = from; idx < usz; ++idx ) {
			const adj_t utadj = untried[idx];
			size_t sidx = untried[idx].first;

			cellmap[utadj] = OCCUPIED;

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
			cellmap[utadj] = REACHABLE;
			untried.resize( usz );
		}

		for( size_t idx = from; idx < usz; ++idx ) {
			cellmap.erase( untried[idx] );
		}

		return total;
	}
}
