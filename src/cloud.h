#pragma once

#include <vector>
#include <list>
#include <bitset>

#include "shape.h"
#include "bitmap.h"

enum Orientations
{
	TRANSLATIONS_ONLY,
	TRANSLATIONS_ROTATIONS,
	ALL
};

template<typename grid>
struct Orientation
{
	using xform_t = typename grid::xform_t;

	Orientation( const xform_t& T, 
		const Shape<grid>& shape, 
		const Shape<grid>& halo, 
		const Shape<grid>& border )
		: T_ { T }
		, shape_ { shape }
		, halo_ { halo }
		, border_ { border }
	{}
	
	xform_t T_;
	Shape<grid> shape_;
	Shape<grid> halo_;
	Shape<grid> border_;
};

// The cloud is the set of all transforms that relate to the central copy of a
// shape.  Each transform can either be overlapping, cleanly adjacent, or
// adjacent but not simply connected.

// NOTE: seriously: need helper functions to test a transformed tile T
// relative to another transformed tile S, so that we don't have to keep
// rederiving the relationship.  Sheesh.  Actually, this may work better
// as a method of xform.

template<typename grid>
class Cloud
{
public:
	using coord_t = typename grid::coord_t;
	using xform_t = typename grid::xform_t;
	using point_t = typename grid::point_t;
	using bitgrid_t = bitgrid<128>;

	Cloud( const Shape<grid>& shape, Orientations ori = ALL, bool filterSymmetries = false, bool reduce = false );

	bool isOverlap( const xform_t& T ) const
	{
		return overlapping_.find( T ) != overlapping_.end();
	}
	bool isAdjacent( const xform_t& T ) const
	{
		return adjacent_.find( T ) != adjacent_.end();
	}
	bool isCulledAdjacent( const xform_t& T ) const
	{
		return adjacent_culled_.find( T ) != adjacent_culled_.end();
	}
	bool isHoleFreeAdjacent(const xform_t& T) const
	{
		return (adjacent_.find(T) != adjacent_.end()) ||
			(adjacent_culled_.find(T) != adjacent_culled_.end());
	}

	bool isHoleAdjacent( const xform_t& T ) const
	{
		return adjacent_hole_.find( T ) != adjacent_hole_.end();
	}
	bool isAnyAdjacent( const xform_t& T ) const
	{
		return isAdjacent( T ) || isHoleAdjacent( T );
	}
	bool isAny( const xform_t& T ) const
	{
		return isOverlap( T ) || isAdjacent( T ) || isHoleAdjacent( T );
	}

	void calcOrientations( Orientations ori );
	void calcUniqueOrientations( Orientations ori );

	bool checkSimplyConnectedV1( const xform_t& T ) const;
	bool checkSimplyConnectedV2( bitgrid_t& bits, const xform_t& T ) const;
	bool checkSimplyConnected( bitgrid_t bits, const xform_t& T ) const;

	void reduceAdjacents();
	template<typename bitset> void reduceAdjacentsImpl();

	void debug( std::ostream& os ) const;
	void debugTransform( std::ostream& os, const xform_t& T ) const;

	Shape<grid> shape_;
	Shape<grid> halo_;
	Shape<grid> border_;

	std::vector<Orientation<grid>> orientations_;

	// The main set of adjacents to use in surround computations,
	// possibly reduced.
	xform_set<coord_t> adjacent_;
	// The set of adjacents that might have been pulled out of the set
	// above by reduction.  Possibly empty.
	xform_set<coord_t> adjacent_culled_;

	xform_set<coord_t> adjacent_hole_;
	xform_set<coord_t> overlapping_;
	bool surroundable_;
	bool reduced_surroundable_;
};

template<typename grid>
Cloud<grid>::Cloud( const Shape<grid>& shape, Orientations ori, 
	bool filterSymmetries, bool reduce )
	: shape_ { shape }
	, adjacent_ {}
	, adjacent_culled_ {}
	, adjacent_hole_ {}
	, overlapping_ {}
	, surroundable_ { true }
	, reduced_surroundable_ { true }
{
	shape.getHaloAndBorder(halo_, border_);

	if (!filterSymmetries) {
		calcOrientations(ori);
	} else {
		calcUniqueOrientations(ori);
	}

	// Overlaps are easy to detect -- there must be a cell that's covered
	// by a border cell of both transformed copies of the shape.  This
	// incurs some significant redundancy, but makes subsequent adjacency
	// checking faster by avoiding explicit intersection tests.

	// NOTE -- is there an cheaper way to do this?  Is it possible that
	// the space of overlaps for a fixed orientation can be generated 
	// from some kind of DFS?
	for( auto& bp : border_ ) {
		for( auto& ori : orientations_ ) {
			for( auto& obp : ori.border_ ) {
				if( grid::translatable( obp, bp ) ) {
					xform_t Tnew = ori.T_.translate( bp - obp );
					// Avoid storing the identity matrix.
					if( !Tnew.isIdentity() ) {
						overlapping_.insert( Tnew );
					}
				}
			}
		}
	}

	bitgrid_t bits;

	// Initialize the bitgrid to the halo of the base shape.
	for( const auto& p : halo_ ) {
		bits.set( p, 1 );
	}

	// Now try to construct all adjacencies by translating a border
	// point of an oriented shape to a halo point of the main shape.

	// NOTE: There may be some value in immediately distinguishing between
	// Adjacencies that share a single point in common vs. adjacencies
	// that share one or more edges.  That would in turn rely on 
	// precomputing the shape's edge halo (see Shape::getEdgeHalo)
	for( auto hp : halo_ ) {
		bool found = false;

		for( auto& ori : orientations_ ) {
			for( auto& tbp : ori.border_ ) {
				if( !grid::translatable( hp, tbp ) ) {
					continue;
				}

				xform_t Tnew = ori.T_.translate( hp - tbp );

				if( isOverlap( Tnew ) ) {
					// Computed previously, keep moving.
					continue;
				}

				if( isAdjacent( Tnew ) ) {
					found = true;
					continue;
				} else if( isHoleAdjacent( Tnew ) ) {
					continue;
				}

				// We previously ruled out every possible overlap,
				// so we know this new tile is adjacent.  But the adjacency
				// might not be simply connected.

				bool sc = checkSimplyConnected( bits, Tnew );

/*
				bool sc_validation = checkSimplyConnectedV1( Tnew );
				if( sc != sc_validation ) {
					std::cerr << "Simply connected check failed validation!"
							  << std::endl;
				}
				*/

				if( sc ) {
					found = true;

					adjacent_.insert( Tnew );
					// It's convenient to throw T inverse into the
					// list of adjacents as well, so that we don't
					// waste computation on it later when it comes 
					// up during iteration.  But that doesn't work
					// when you're using unique orientations (i.e.,
					// factoring out symmetry), because T inverse
					// might be one of the orientations you previously
					// factored out.  So just skip it in that case.
					if (!filterSymmetries) {
						adjacent_.insert( Tnew.invert() );
					}
				} else {
					adjacent_hole_.insert( Tnew );
					if (!filterSymmetries) {
						adjacent_hole_.insert( Tnew.invert() );
					}
				}
			}
		}

		// If there's a halo cell with no legal adjacency, Heesch numbers
		// definitely don't work.  So don't bother doing any more work, 
		// just stop here.
		if( !found ) {
			surroundable_ = false;
			return;
		}
	}

	if( reduce ) {
		reduceAdjacents();
	}
}

// Check naively whether the base tile and a copy transformed
// by T form a simply connected patch.  This is an expensive 
// operation, and should only be used as validation of the
// correctness of faster algorithms.
template<typename grid>
bool Cloud<grid>::checkSimplyConnectedV1( const xform_t& T ) const
{
	Shape<grid> new_shape;

	new_shape.reset( shape_, T );
	new_shape.add( shape_ );

	return new_shape.simplyConnected();
}

// A faster connectedness check.  Record the union of the haloes
// of the tile and its transformed copy, then subtract the tiles
// themselves, then check whether the remaining halo is 
// (edge-)connected.  This version is deprecated and can eventually
// be deleted.
template<typename grid>
bool Cloud<grid>::checkSimplyConnectedV2( 
	bitgrid_t& bits, const xform_t& T ) const
{
	bits.clear();

	size_t halo_size = 0;

	// Add both halos
	for( const auto& p : halo_ ) {
		// std::cerr << "Setting " << p << " as halo" << std::endl;
		if( bits.getAndSet( p, 1 ) == 0 ) {
			++halo_size;
		}
		// std::cerr << "Setting " << (T * p) << " as halo" << std::endl;
		if( bits.getAndSet( T * p, 1 ) == 0 ) {
			++halo_size;
		}
	}

	// Subtract shapes
	for( const auto& p : shape_ ) {
		// std::cerr << "Setting " << p << " as empty" << std::endl;
		if( bits.getAndSet( p, 0 ) != 0 ) {
			--halo_size;
		}
		// std::cerr << "Setting " << (T * p) << " as empty" << std::endl;
		if( bits.getAndSet( T * p, 0 ) != 0 ) {
			--halo_size;
		}
	}

	// bits.debug();

	// Check if the union halo is connected using edge adjacencies of the cell
	// tiling (see also shape.h).

	// A stack for DFS.  We don't expect the stack to grow very tall in practice.
	// Probably 64 is overkill.
	point_t work[64];

	// Initialize the stack with any halo cell
	for( const auto& p : halo_ ) {
		if( bits.get( p ) != 0 ) {
			work[0] = p;
			break;
		}
	}

	size_t num_visited = 0;
	size_t size = 1;

	while( size > 0 ) {
		auto p = work[size-1];
		// std::cerr << "Visiting " << p << std::endl;
		--size;

		if( bits.get( p ) != 0 ) {
			++num_visited;
			bits.set( p, 0 );

			for( auto pn : edge_neighbours<grid> { p } ) {
				if( bits.get( pn ) != 0 ) {
					work[size] = pn;
					++size;
				}
			}
		}
	}

	return num_visited == halo_size;
}

// A refined version of checkSimplyConnectedV2().  Add only the 
// halo of the base shape, and subtract only the transformed tile.
// This improved routine has undergone a reasonable amount of testing
// and agrees with reference implementation in all cases.
template<typename grid>
bool Cloud<grid>::checkSimplyConnected( bitgrid_t bits, const xform_t& T ) const
{
	// Deliberately pass bits by value, so that we get a freshly
	// initialized copy.

	size_t halo_size = halo_.size();

	// Subtract shape
	for( const auto& p : shape_ ) {
		if( bits.getAndSet( T * p, 0 ) != 0 ) {
			--halo_size;
		}
	}

	point_t work[256];

	// Initialize the stack with any halo cell
	for( const auto& p : halo_ ) {
		if( bits.get( p ) != 0 ) {
			work[0] = p;
			break;
		}
	}

	size_t num_visited = 0;
	size_t size = 1;

	while( size > 0 ) {
		auto p = work[size-1];
		--size;

		if( bits.get( p ) != 0 ) {
			++num_visited;
			bits.set( p, 0 );

			// NOTE -- these neighbours could be precomputed
			// as an adjacency list above, so that we don't have
			// to spend time checking the bitmap in places
			// that are known to be empty.
			for( auto pn : edge_neighbours<grid> { p } ) {
				if( bits.get( pn ) != 0 ) {
					work[size] = pn;
					++size;
				}
			}
		}
	}

	return num_visited == halo_size;
}

template<typename grid>
template<typename bitset>
void Cloud<grid>::reduceAdjacentsImpl()
{
	using xform_info = std::pair<xform_t, bitset>;

	size_t sz = adjacent_.size();
	size_t cur_size = 0;
	xform_info *cur = new xform_info[sz];
	size_t next_size = 0;
	xform_info *next = new xform_info[sz];

	// NOTE -- would be nice to get rid of this expensive point_map
	// data structure.
	point_map<coord_t, bitset> halo;
	halo.reserve(halo_.size());
	size_t idx = 0;
	for (const auto& p: halo_) {
		halo[p].set(idx);
		++idx;
	}

	for (const auto& T: adjacent_) {
		bitset occ;
		for (const auto& p: shape_) {
			point_t tp = T * p;
			auto i = halo.find(tp);
			if (i != halo.end()) {
				occ |= i->second;
			}
		}

		cur[cur_size] = std::make_pair(T, occ);
		++cur_size;
	}

	while (true) {
		for (size_t idx = 0; idx < cur_size; ++idx) {
			const xform_t& T = cur[idx].first;
			bitset occ = cur[idx].second;

			// Find all other transforms that don't overlap T,
			// use them to mark the halo. 
			for (size_t jdx = 0; jdx < cur_size; ++jdx) {
				const xform_t& OT = cur[jdx].first;

				if (jdx == idx) {
					continue;
				}
				if (isOverlap(T.invert() * OT)) {
					continue;
				}
				if (isHoleAdjacent(T.invert() * OT)) {
					continue;
				}

				occ |= cur[jdx].second;
			}

			// You'd think that occ.count() is slow, but apparently
			// it can be implemented with a single instructions on modern
			// architectures.  Would be interesting to check whether
			// that's happening.
			if (occ.count() == halo_.size()) {
				// We managed to occupy the whole halo, so keep this
				// neighbour around.
				next[next_size] = cur[idx];
				++next_size;
			} else {
				// Transfer this xform from the main adjacents list 
				// into the culled list.
				adjacent_.erase(cur[idx].first);
				adjacent_culled_.insert(cur[idx].first);
				// std::cout << cur[idx].first << std::endl;
			}
		}

		if (next_size < cur_size) {
			xform_info *tmp = cur;
			cur = next;
			next = tmp;
			cur_size = next_size;
			next_size = 0;

			if (cur_size == 0) {
				break;
			}
		} else {
			break;
		}
	}

	// std::cerr << "Ending with " << cur_size << " adjacents" << std::endl;

	if (cur_size == 0) {
		reduced_surroundable_ = false;
	}

	delete [] cur;
	delete [] next;
}

template<typename grid>
void Cloud<grid>::reduceAdjacents()
{
	// Dispatch to a few compile-time set sizes or give up if the
	// halo is too big.

	if (halo_.size() <= 64) {
		reduceAdjacentsImpl<std::bitset<64>>();
	} else if (halo_.size() <= 256) {
		reduceAdjacentsImpl<std::bitset<256>>();
	} else if (halo_.size() <= 1024) {
		reduceAdjacentsImpl<std::bitset<1024>>();
	} else {
		// NOTE -- could revert to a dynamic bitset here?  Note that in
		// the worst case this is a graceful failure mode -- downstream
		// code won't break, it'll just be slower.
		std::cerr << "Cannot reduce adjacents when halo size is above 1024"
			<< std::endl;
	}
}

template<typename grid>
void Cloud<grid>::calcOrientations( Orientations ori )
{
	// It seems natural to want to factor out symmetric orientations of the
	// shape.  But there are two considerations that get in the way of
	// that:
	// 1. Once we get up to larger polyforms, most shapes are asymmetric,
	//    so you might not really be gaining very much.
	// 2. In higher coronas we might arrive at the same shape placement
	//    via two different concatenations of transformations, yielding
	//    the same transformed shape represented via two different matrices.
	//    Those copies won't find each other, which is bad.

	// Construct oriented copies, factoring out symmetries.
	for( size_t idx = 0; idx < grid::num_orientations; ++idx ) {
		xform_t T { grid::orientations[idx] };

		if( ori == TRANSLATIONS_ONLY ) {
			if( !T.isTranslation() ) {
				continue;
			}
		} else if( ori == TRANSLATIONS_ROTATIONS ) {
			if( T.det() < 0 ) {
				continue;
			}
		}

		Shape<grid> oshape;
		Shape<grid> ohalo;
		Shape<grid> oborder;
		oshape.reset( shape_, T );
		ohalo.reset( halo_, T );
		oborder.reset( border_, T );
		orientations_.emplace_back( T, oshape, ohalo, oborder );
	}

/*
		// Find the smallest cell that's translatable to zero and recenter
		// the shape
		point_t hv;
		for( auto& p : oshape ) {
			if( grid::translatable( p, point_t { 0, 0 } ) ) {
				hv = -p;
				break;
			}
		}

		oshape.translate( hv );

		bool found = false;
		// Now check if any existing orientation is identical to this one.
		for( auto& ori : orientations_ ) {
			if( ori.shape_ == oshape ) {
				found = true;
				break;
			}
		}

		if( !found ) {
			// Must add a new orientation.
			Shape<grid> new_halo;
			Shape<grid> new_border;

			new_halo.reset( halo_, T );
			new_halo.translate( hv );
			new_border.reset( border_, T );
			new_border.translate( hv );

			orientations_.emplace_back( 
				T.translate( hv ), oshape, new_halo, new_border );
		}
	}

	// std::cout << orientations_.size() << " Orientations." << std::endl;
	*/
}

template<typename grid>
void Cloud<grid>::calcUniqueOrientations( Orientations ori )
{
	std::vector<Shape<grid>> uniqueOri;
	Shape<grid> ts;

	// Construct oriented copies, factoring out symmetries.
	for( size_t idx = 0; idx < grid::num_orientations; ++idx )
	{
		xform_t T { grid::orientations[idx] };

		if( ori == TRANSLATIONS_ONLY ) {
			if( !T.isTranslation() ) {
				continue;
			}
		} else if( ori == TRANSLATIONS_ROTATIONS ) {
			if( T.det() < 0 ) {
				continue;
			}
		}

		Shape<grid> oshape;
		Shape<grid> ohalo;
		Shape<grid> oborder;
		oshape.reset( shape_, T );
		ohalo.reset( halo_, T );
		oborder.reset( border_, T );

		// Filter out symmetries
		bool isUnique = true;
		ts.reset( shape_, T );
		ts.untranslate();

		for (const auto & unique : uniqueOri)
		{
			if ( unique == ts )
			{
				isUnique = false;
				break;
			}
		}

		if (isUnique)
		{
			uniqueOri.emplace_back(ts);
			orientations_.emplace_back( T, oshape, ohalo, oborder );
		}
	}
}

template<typename grid>
void Cloud<grid>::debugTransform( std::ostream& os, const xform_t& T ) const
{
	static char buf[1600];
	std::fill( buf, buf + 1600, ' ' );

	int xmin = 40;
	int ymin = 40;
	int xmax = 0;
	int ymax = 0;

	for( auto p : shape_ ) {
		buf[40*(p.y_+20) + (p.x_+20)] = '#';
		xmin = std::min( xmin, int(p.x_ + 20) );
		xmax = std::max( xmax, int(p.x_ + 20) );
		ymin = std::min( ymin, int(p.y_ + 20) );
		ymax = std::max( ymax, int(p.y_ + 20) );
	}

	for( auto p : shape_ ) {
		point_t tp = T * p;
		buf[40*(tp.y_+20) + (tp.x_+20)] = 'O';
		xmin = std::min( xmin, int(tp.x_ + 20) );
		xmax = std::max( xmax, int(tp.x_ + 20) );
		ymin = std::min( ymin, int(tp.y_ + 20) );
		ymax = std::max( ymax, int(tp.y_ + 20) );
	}

	for( size_t y = ymin; y <= ymax; ++y ) {
		for( size_t x = xmin; x <= xmax; ++x ) {
			os << buf[y*40+x];
		}
		os << std::endl;
	}
	os << std::endl;
}

template<typename grid>
void Cloud<grid>::debug( std::ostream& os ) const
{
	os << "Adjacent: " << adjacent_.size() << std::endl;
	os << "Hole Adjacent: " << adjacent_hole_.size() << std::endl;
	os << "Overlapping: " << overlapping_.size() << std::endl;

	os << "=========== OVERLAPPING ============" << std::endl;
	for( auto & T : overlapping_ ) {
		// debugTransform( os, T );
		os << "  " << T << std::endl;
	}

	os << "=========== ADJACENT ============" << std::endl;
	for( auto & T : adjacent_ ) {
		// debugTransform( os, T );
		os << "  " << T << std::endl;
	}

	os << "=========== HOLE ADJACENT ============" << std::endl;
	for( auto & T : adjacent_hole_ ) {
		// debugTransform( os, T );
		os << "  " << T << std::endl;
	}
}
