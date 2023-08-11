#pragma once

#include "geom.h"

#include "ominogrid.h"
#include "hexgrid.h"
#include "iamondgrid.h"
#include "octasquaregrid.h"
#include "trihexgrid.h"
#include "abologrid.h"
#include "draftergrid.h"
#include "kitegrid.h"

enum GridType {
	OMINO,
	HEX,
	IAMOND,
	OCTASQUARE,
	TRIHEX,
	ABOLO,
	DRAFTER, 
	KITE
};

inline GridType getGridType( int& argc, char **argv )
{
	static const std::pair<const char *, GridType> types[] = {
		{ "-omino", OMINO }, 
		{ "-hex", HEX }, 
		{ "-iamond", IAMOND },
		{ "-octasquare", OCTASQUARE },
		{ "-kite", KITE },
	};

	size_t idx = 1; 
	GridType grid = OMINO; 
	bool found = false;
	while( idx < argc ) {  
		for( auto& p : types ) {
			if( !strcmp( argv[idx], p.first ) ) {
				grid = p.second;
				found = true;
				break;
			}
		}
		if( found ) {
			break;
		}
		++idx;
	}

	++idx; 
	while( idx < argc ) { 
		argv[idx-1] = argv[idx]; 
		++idx; 
	} 

	--argc;
	return grid;
}

// Ugh!  Surely there's a more elegant, non-macro way to make this work?
#define bootstrap_grid( argc, argv, func ) \
	{ \
		GridType grid = getGridType( argc, argv ); \
		if( grid == OMINO ) { \
			func<OminoGrid<int16_t>>( argc, argv ); \
		} else if( grid == HEX ) { \
			func<HexGrid<int16_t>>( argc, argv ); \
		} else if( grid == IAMOND ) { \
			func<IamondGrid<int16_t>>( argc, argv ); \
		} else if( grid == OCTASQUARE ) { \
			func<OctaSquareGrid<int16_t>>( argc, argv ); \
		} else if( grid == KITE ) { \
			func<KiteGrid<int16_t>>( argc, argv ); \
		} \
	}

template<typename grid>
struct neighbour_maker
{
	using point_t = typename grid::point_t;

	struct neighbour_iter
	{
		neighbour_iter()
			: pt_ {}
			, idx_ { 0 }
			, pts_ { nullptr }
		{}
		neighbour_iter( const point_t p, size_t i, const point<int8_t>* pts )
			: pt_ { p }
			, idx_ { i }
			, pts_ { pts }
		{}

		point_t operator *()
		{
			return pt_ + pts_[idx_];
		}

		bool operator ==( const neighbour_iter& other ) const
		{
			//return (pt_ == other.pt_) && (idx_ == other.idx_);
			return idx_ == other.idx_;
		}
		bool operator !=( const neighbour_iter& other ) const
		{
			// return (pt != other.pt) || (idx != other.idx);
			return idx_ != other.idx_;
		}

		neighbour_iter& operator++()
		{
			++idx_;
			return *this;
		}
		neighbour_iter operator++( int )
		{
			neighbour_iter ret { *this };
			++idx_;
			return ret;
		}

		point_t pt_;
		size_t idx_;
		const point<int8_t> *pts_;
	};

	neighbour_maker( const point_t& p )
		: pt_ { p }
	{}

	point_t pt_;
};

template<typename grid>
struct neighbours 
	: public neighbour_maker<grid>
{
	using point_t = typename grid::point_t;
	using iter = typename neighbour_maker<grid>::neighbour_iter;

	neighbours( const point_t& p )
		: neighbour_maker<grid> { p }
	{}

	iter begin()
	{
		return iter { 
			this->pt_, 
			0, 
			grid::getNeighbourVectors( this->pt_ ) };
	}
	iter end()
	{
		return iter {
			this->pt_, 
			grid::numNeighbours( this->pt_ ), 
			grid::getNeighbourVectors( this->pt_ ) };
	}
};

template<typename grid>
struct edge_neighbours 
	: public neighbour_maker<grid>
{
	using point_t = typename grid::point_t;
	using iter = typename neighbour_maker<grid>::neighbour_iter;

	edge_neighbours( const point_t& p )
		: neighbour_maker<grid> { p }
	{}

	iter begin()
	{
		return iter {
			this->pt_,
			0, 
			grid::getEdgeNeighbourVectors( this->pt_ ) };
	}
	iter end()
	{
		return iter {
			this->pt_, 
			grid::numEdgeNeighbours( this->pt_ ), 
			grid::getEdgeNeighbourVectors( this->pt_ ) };
	}
};
