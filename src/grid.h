#pragma once

#include <utility>

#include "geom.h"

#include "ominogrid.h"
#include "hexgrid.h"
#include "iamondgrid.h"
#include "octasquaregrid.h"
#include "trihexgrid.h"
#include "abologrid.h"
#include "draftergrid.h"
#include "kitegrid.h"
#include "halfcairogrid.h"
#include "bevelhexgrid.h"

// There's no generic "Grid" base class -- because we're implementing
// all grids as template parameters, we're basically relying on structural
// polymorphism with no common behaviour.  Nevertheless, there are a 
// few functions and types that apply across all grids.  Define them here.

// Pull a grid type out of command line arguments, and splice that
// argument out.
inline GridType getGridType( int& argc, char **argv )
{
	static const std::pair<const char *, GridType> types[] = {
		{ "-omino", OMINO }, 
		{ "-hex", HEX }, 
		{ "-iamond", IAMOND },
		{ "-octasquare", OCTASQUARE },
		{ "-trihex", TRIHEX },
		{ "-kite", KITE },
		{ "-drafter", DRAFTER },
		{ "-abolo", ABOLO },
		{ "-halfcairo", HALFCAIRO },
		{ "-bevelhex", BEVELHEX },
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

// Set up a few tools to support run-time dispatch based on grid type.
// This turns out to be a particularly fussy bit of C++.  I couldn't
// figure out how to make it work purely using template metaprogramming,
// so I resorted to a couple of macros and a bit too much explicit code
// in the bootstrapping code of the various programs in this system.

template<template<typename grid> class Func, typename... Args>
auto dispatchToGridType( GridType gt, Args ...args )
{
	using coord = int16_t;

	switch( gt ) {
		case HEX: return Func<HexGrid<coord>>()( args... ); 
		case IAMOND: return Func<IamondGrid<coord>>()( args... ); 
		case KITE: return Func<KiteGrid<coord>>()( args... ); 
		case DRAFTER: return Func<DrafterGrid<coord>>()( args... ); 
		case ABOLO: return Func<AboloGrid<coord>>()( args... ); 
		case OCTASQUARE: return Func<OctaSquareGrid<coord>>()( args... ); 
		case TRIHEX: return Func<TriHexGrid<coord>>()( args... ); 
		case HALFCAIRO: return Func<HalfCairoGrid<coord>>()( args... ); 
		case BEVELHEX: return Func<BevelHexGrid<coord>>()( args... ); 
		case OMINO: default: return Func<OminoGrid<coord>>()( args... ); 
	} 
}

#define GRID_WRAP( f ) \
template<typename grid> \
struct f##Wrapper \
{ \
public: \
	template<typename... Args> \
	auto operator()( Args ...args ) \
	{ \
		return f<grid>( args... ); \
	} \
}

#define GRID_DISPATCH( f, gt, ... ) \
	dispatchToGridType<f##Wrapper>( gt, __VA_ARGS__ )

// Utility structures that eat grids and spit out iterators.

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
