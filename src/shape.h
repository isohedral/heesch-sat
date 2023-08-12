#pragma once

#include <list>
#include <stack>
#include <algorithm>

#include "geom.h"
#include "grid.h"

template<typename grid>
class Shape
{
public:
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;
	using xform_t = typename grid::xform_t;

	Shape()
		: pts_ {}
	{}
	Shape( const Shape<grid>& other )
		: pts_ { other.pts_ }
	{}
	Shape( const Shape<grid>& other, const point_t& v )
	{
		for( auto p : other ) {
			pts_.push_back( p + v );
		}
	}

	size_t size() const
	{
		return pts_.size();
	}

	void add( const point_t& p )
	{
		pts_.push_back( p );
	}
	void add( const Shape<grid>& other )
	{
		for( auto p : other ) {
			pts_.push_back( p );
		}
	}

	void complete()
	{
		pts_.sort();
	}

	void reset()
	{
		pts_.clear();
	}
	void reset( const Shape& other, const xform_t& T )
	{
		pts_.clear();
		for( auto p : other ) {
			pts_.push_back( T * p );
		}
		complete();
	}
	void translate( const point_t& dp )
	{
		// Translating should never affect the order.
		for( auto& p : pts_ ) {
			p += dp;
		}
	}

	bool intersects( const Shape<grid>& other ) const;
	bool operator ==( const Shape<grid>& other ) const
	{
		return std::equal(
			pts_.begin(), pts_.end(), other.pts_.begin(), other.pts_.end() );
	}
	bool operator !=( const Shape<grid>& other ) const
	{
		return (*this) != other;
	}

	void getHaloAndBorder( Shape<grid>& halo, Shape<grid>& border ) const;
	bool simplyConnected() const;

	auto begin()
	{ return pts_.begin(); }
	auto end()
	{ return pts_.end(); }

	auto begin() const
	{ return pts_.begin(); }
	auto end() const
	{ return pts_.end(); }

	void debug() const;

private:
	std::list<point_t> pts_;
};

// To use this method, both shapes must be "complete" (i.e., sorted)
template<typename grid>
bool Shape<grid>::intersects( const Shape<grid>& other ) const
{
	auto i = begin();
	auto j = other.begin();

	// Merge-like algorithm

	while( true ) {
		if( (i == end()) || (j == other.end()) ) {
			return false;
		} else if( *i == *j ) {
			return true;
		} else if( *i < *j ) {
			++i;
		} else {
			++j;
		}
	}
}

template<typename grid>
void Shape<grid>::getHaloAndBorder( Shape<grid>& halo, Shape<grid>& border ) const
{
	halo.reset();
	border.reset();

	point_map<coord_t,size_t> counts;

	for( auto& p : pts_ ) {
		++counts[p];
		for( auto pn : neighbours<grid> { p } ) {
			++counts[pn];
		}
	}

	for( auto& p : pts_ ) {
		if( counts[p] < grid::numNeighbours( p ) + 1 ) {
			border.add( p );
		}

		counts.erase( p );
	}

	for( auto& i : counts ) {
		halo.add( i.first );
	}

	halo.complete();
	border.complete();
}

// Does this shape contain any internal holes?
template<typename grid>
bool Shape<grid>::simplyConnected() const
{
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;

	Shape<grid> halo;
	Shape<grid> border;
	getHaloAndBorder( halo, border );
	
	/*
	std::cerr << "Halo:";
	for( auto p : halo ) {
		std::cerr << " " << p;
	} 
	std::cerr << std::endl;
	*/

	// Make it easy to look things up in the halo
	point_set<coord_t> halo_set;
	for( auto p : halo ) {
		halo_set.insert( p );
	}

	// A shape is simply connected if its halo is a single connected component
	// (using edge neighbours).  So take the first halo cell and try to visit 
	// the entire halo from it.

	point_set<coord_t> visited;
	std::stack<point_t> working;
	working.push( halo.pts_.front() );

	while( !working.empty() ) {
		auto p = working.top();
		working.pop();

		if( visited.find( p ) == visited.end() ) {
			// Haven't visited this point yet.
			visited.insert( p );

			for( auto pn : edge_neighbours<grid> { p } ) {
				if( halo_set.find( pn ) != halo_set.end() ) {
					// Found an adjacent halo cell.  Plan to visit it.

					// This cell may already have been visited, but
					// there's no harm in sticking it in the working
					// set and removing it again later (except for a 
					// bit of memory pressure...)
					working.push( pn );
				}
			}
		}
	}

	return visited.size() == halo.size();
}

template<typename grid>
void Shape<grid>::debug() const
{
	for( auto& p : pts_ ) {
		std::cerr << " " << p;
	}
	std::cerr << std::endl;
}
