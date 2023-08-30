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

	void add( const coord_t& x, const coord_t& y )
	{
		pts_.emplace_back( x, y );
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

	Shape& operator =( const Shape& other ) 
	{
		if( pts_.size() == other.pts_.size() ) {
			std::copy( other.pts_.begin(), other.pts_.end(), pts_.begin() );
		} else {
			pts_.clear();
			std::copy( other.pts_.begin(), other.pts_.end(), 
				std::back_inserter( pts_ ) );
		}
		return *this;
	}

	void reset()
	{
		pts_.clear();
	}
	void reset( const Shape& other, const xform_t& T )
	{
		if( pts_.size() == other.pts_.size() ) {
			// If same size, overwrite without reallocating.
			auto i = pts_.begin();
			auto j = other.pts_.begin();
			while( i != pts_.end() ) {
				*i = T * (*j);
				++i;
				++j;
			}
		} else {
			pts_.clear();
			for( auto p : other ) {
				pts_.push_back( T * p );
			}
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
	// Reset with translation
	void reset( const Shape& other, const point_t& dp )
	{
		pts_.clear();
		for( auto p : other ) {
			pts_.push_back( p + dp );
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

	void getSymmetries( std::vector<xform_t>& syms ) const
	{
		Shape<grid> other;
		syms.clear();
		for( size_t idx = 1; idx < grid::num_orientations; ++idx ) {
			other.reset( *this, grid::orientations[idx] );
			if( equivalent( other ) ) {
				syms.push_back( grid::orientations[idx] );
			}
		}
	}

	// Are these shapes equivalent under translation?
	bool equivalent( const Shape<grid>& other ) const
	{
		if( size() != other.size() ) {
			return false;
		}

		if( !grid::translatable( pts_.front(), other.pts_.front() ) ) {
			return false;
		}
		
		point_t d = pts_.front() - other.pts_.front();

		auto i = pts_.begin();
		auto j = other.pts_.begin();

		while( i != pts_.end() ) {
			if( (*i) != ((*j)+d) ) {
				return false;
			}
			++i;
			++j;
		}

		return true;
	}

	// Move this shape so its minimum point lies at an origin of the grid.
	void untranslate()
	{
		point_t p = pts_.front();
		point_t v = grid::getOrigin( p ) - p;

		for( auto& sp : pts_ ) {
			sp += v;
		}
	}

	int compare( const Shape<grid>& other ) const
	{
		auto i = pts_.begin();
		auto j = other.pts_.begin();

		while( true ) {
			bool ei = (i == pts_.end());
			bool ej = (j == other.pts_.end());

			if( ei && ej ) {
				return 0;
			} else if( ei ) {
				return -1;
			} else if( ej ) {
				return 1;
			} else if( *i < *j ) {
				return -1;
			} else if( *j < *i ) {
				return 1;
			}

			++i;
			++j;
		}
	}
	bool operator <( const Shape<grid>& other ) const
	{
		return compare( other ) < 0;
	}

	void getHaloAndBorder( Shape<grid>& halo, Shape<grid>& border ) const;
	void getEdgeHalo( Shape<grid>& halo ) const;
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

// Like the halo above, but only includes neighbours across edges, not
// vertices.
template<typename grid>
void Shape<grid>::getEdgeHalo( Shape<grid>& halo ) const
{
	halo.reset();

	point_set<coord_t> shalo;

	for( auto& p : pts_ ) {
		for( const auto& pn : edge_neighbours<grid> { p } ) {
			shalo.insert( pn );
		}
	}

	for( auto& p : pts_ ) {
		shalo.erase( p );
	}

	for( auto& p : shalo ) {
		halo.add( p );
	}

	halo.complete();
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
				// std::cerr << pn << " is an edge neighbour of " << p << std::endl;
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

/*
	debug();
	if( visited.size() == halo.size() ) {
		std::cerr << " ... is simply connected" << std::endl;
	} else {
		std::cerr << " ... is NOT simply connected" << std::endl;
	}
*/

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
