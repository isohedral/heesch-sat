#pragma once

#include <vector>
#include <stack>
#include <unordered_set>

#include "grid.h"
#include "shape.h"

template<typename grid>
class HoleFinder
{
public:
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;
	using xform_t = typename grid::xform_t;

	HoleFinder( const Shape<grid>& shape );
	void addCopy( tile_index idx, const xform_t& T );
	bool getHoles( std::vector<std::vector<tile_index>>& holes );

private:
	void computeHalo();
	bool search( const point_t& cell, point_set<coord_t>& visited,
		std::unordered_set<tile_index>& owners ) const;

	const Shape<grid>& shape_;
	point_map<coord_t,tile_index> cells_;

	point_set<coord_t> halo_;
	point_t halo_min_;
};

template<typename grid>
HoleFinder<grid>::HoleFinder( const Shape<grid>& shape )
	: shape_ { shape }
	, cells_ {}
{}

template<typename grid>
void HoleFinder<grid>::addCopy( tile_index idx, const xform_t& T )
{
	for( auto& p: shape_ ) {
		cells_[ T * p ] = idx;
	}
}

template<typename grid>
void HoleFinder<grid>::computeHalo()
{
	halo_.clear();
	bool start = true;

	for( auto i : cells_ ) {
		point_t p = i.first;

		for( auto pn : neighbours<grid> { p } ) {
			// If pn isn't already a cell, add it with index -1 to
			// indicate a halo cell.
			if( cells_.find( pn ) == cells_.end() ) {
				halo_.insert( pn );

				// Track the minimum halo point, which is guaranteed
				// to lie on the outer boundary.
				if( start || (pn < halo_min_) ) {
					halo_min_ = pn;
					start = false;
				}
			}
		}
	}
}

template<typename grid>
bool HoleFinder<grid>::search( const point_t& cell, 
	point_set<coord_t>& visited, std::unordered_set<tile_index>& owners ) const
{
	std::stack<point_t> working;
	working.push( cell );
	bool was_outer = false;

	while( !working.empty() ) {
		auto p = working.top();
		working.pop();

		if( visited.find( p ) == visited.end() ) {
			// Haven't visited this point yet.
			visited.insert( p );

			if( p == halo_min_ ) {
				// Part of this search turned up a cell known to be on
				// the outer halo ring, which is definitely not a hole.
				// Make a note of this, but you must continue exploring
				// the whole component.
				was_outer = true;
			}

			// Continue on to all edge neighbours that are halo cells.
			for( auto pn : edge_neighbours<grid> { p } ) {
				if( halo_.find( pn ) != halo_.end() ) {
					// This is an adjacent halo cell, so continue the search.
					working.push( pn );
				}
			}

			// Check for hole boundary cells
			for( auto pn : neighbours<grid> { p } ) {
				auto i = cells_.find( pn );
				if( i != cells_.end() ) {
					owners.insert( i->second );
				}
			}
		}
	}

	return !was_outer;
}

template<typename grid>
bool HoleFinder<grid>::getHoles( std::vector<std::vector<tile_index>>& holes )
{
	computeHalo();
	holes.clear();

	// We know about halo cells, so now search them all, extracting connected
	// components, and recording hole owners as needed.

	point_set<coord_t> visited;
	bool found_one = false;

	for( auto& p: halo_ ) {
		if( visited.find( p ) == visited.end() ) {
			// Another connected component to visit
			std::unordered_set<tile_index> owners;
			if( search( p, visited, owners ) ) {
				// It was a hole!
				found_one = true;
				holes.emplace_back( owners.begin(), owners.end() );
			}
		}
	}

	return found_one;
}
