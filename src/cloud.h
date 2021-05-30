#pragma once

#include <vector>

#include "shape.h"

// The cloud is the set of all transforms that relate to the central copy of a
// shape.  Each transform can either be overlapping, cleanly adjacent, or
// adjacent but not simply connected.

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

template<typename grid>
class Cloud
{
public:
	using coord_t = typename grid::coord_t;
	using xform_t = typename grid::xform_t;
	using point_t = typename grid::point_t;

	Cloud( const Shape<grid>& shape );

	bool isOverlap( const xform_t& T ) const
	{
		return overlapping_.find( T ) != overlapping_.end();
	}
	bool isAdjacent( const xform_t& T ) const
	{
		return adjacent_.find( T ) != adjacent_.end();
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

	void calcOrientations();

	void debug( std::ostream& os ) const;
	void debugTransform( std::ostream& os, const xform_t& T ) const;

	Shape<grid> shape_;
	Shape<grid> halo_;
	Shape<grid> border_;

	std::vector<Orientation<grid>> orientations_;

	xform_set<coord_t> adjacent_;
	xform_set<coord_t> adjacent_hole_;
	xform_set<coord_t> overlapping_;
	bool surroundable_;
};

template<typename grid>
Cloud<grid>::Cloud( const Shape<grid>& shape )
	: shape_ { shape }
{
	surroundable_ = true;

	shape.getHaloAndBorder( halo_, border_ );
	calcOrientations();

	Shape<grid> new_shape;

	// Overlaps are easy to detect -- there must be a cell that's covered
	// by a border cell of both transformed copies of the shape.  This
	// incurs some significant redundancy, but makes subsequent adjacency
	// checking faster by avoiding explicit intersection tests.
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

	// Now try to construct all adjacencies by translating a border
	// point of an oriented shape to a halo point of the main shape.
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

/*
				if( isAnyAdjacent( Tnew ) ) {
					// Already seen, but offers a potential adjacency for
					// this halo cell.  Record that fact.
					found = true;
					continue;
				}
			*/
				if( isAdjacent( Tnew ) ) {
					found = true;
					continue;
				} else if( isHoleAdjacent( Tnew ) ) {
					continue;
				}

				// We previously ruled out every possible overlap,
				// so we know this new tile is adjacent.  But the adjacency
				// might not be simply connected.
				new_shape.reset( shape_, Tnew );
				new_shape.add( shape_ );

				if( new_shape.simplyConnected() ) {
					found = true;

					adjacent_.insert( Tnew );
					adjacent_.insert( Tnew.invert() );
				} else {
					adjacent_hole_.insert( Tnew );
					adjacent_hole_.insert( Tnew.invert() );
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

	// debug( std::cout );
}

template<typename grid>
void Cloud<grid>::calcOrientations()
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
		debugTransform( os, T );
		// os << "  " << T << std::endl;
	}

	os << "=========== ADJACENT ============" << std::endl;
	for( auto & T : adjacent_ ) {
		debugTransform( os, T );
		// os << "  " << T << std::endl;
	}

	os << "=========== HOLE ADJACENT ============" << std::endl;
	for( auto & T : adjacent_hole_ ) {
		debugTransform( os, T );
		// os << "  " << T << std::endl;
	}
}
