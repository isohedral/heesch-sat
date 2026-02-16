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

	int idx = 1; 
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

template<auto F, typename... Args>
auto dispatchGridTypeImpl(GridType gt, Args&&... args)
{
	using coord = int16_t;

	switch( gt ) {
		case HEX: return F.template operator()<HexGrid<coord>>(
			std::forward<Args>(args)...);
		case IAMOND: return F.template operator()<IamondGrid<coord>>(o
			std::forward<Args>(args)...);
		case KITE: return F.template operator()<KiteGrid<coord>>(
			std::forward<Args>(args)...);
		case DRAFTER: return F.template operator()<DrafterGrid<coord>>(
			std::forward<Args>(args)...);
		case ABOLO: return F.template operator()<AboloGrid<coord>>(
			std::forward<Args>(args)...);
		case OCTASQUARE: return F.template operator()<OctaSquareGrid<coord>>(
			std::forward<Args>(args)...);
		case TRIHEX: return F.template operator()<TriHexGrid<coord>>(
			std::forward<Args>(args)...);
		case HALFCAIRO: return F.template operator()<HalfCairoGrid<coord>>(
			std::forward<Args>(args)...);
		case BEVELHEX: return F.template operator()<BevelHexGrid<coord>>(
			std::forward<Args>(args)...);
		case OMINO: default: return F.template operator()<OminoGrid<coord>>(
			std::forward<Args>(args)...);
	}
}

#define dispatchGrid(f, gt, ...) \
    dispatchGridTypeImpl<[]<typename Grid>(auto&&... args) { \
		f<Grid>(std::forward<decltype(args)>(args)...); \
	}>(gt, __VA_ARGS__)

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

	neighbours(const point_t& p)
		: neighbour_maker<grid> {p}
		, vecs_ {grid::getNeighbourVectors(p)}
	{}

	iter begin()
	{
		return iter {this->pt_, 0, vecs_};
	}
	iter end()
	{
		return iter {this->pt_, grid::numNeighbours(this->pt_), vecs_};
	}

	const point<int8_t> *vecs_;
};

template<typename grid>
struct edge_neighbours 
	: public neighbour_maker<grid>
{
	using point_t = typename grid::point_t;
	using iter = typename neighbour_maker<grid>::neighbour_iter;

	edge_neighbours(const point_t& p)
		: neighbour_maker<grid> {p}
		, vecs_ {grid::getEdgeNeighbourVectors(p)}
	{}

	iter begin()
	{
		return iter {this->pt_, 0, vecs_};
	}
	iter end()
	{
		return iter {this->pt_, grid::numEdgeNeighbours(this->pt_), vecs_};
	}

	const point<int8_t> *vecs_;
};

template<typename grid>
struct vertices
	: public neighbour_maker<grid>
{
	using point_t = typename grid::point_t;
	using iter = typename neighbour_maker<grid>::neighbour_iter;

	vertices(const point_t& p)
		: neighbour_maker<grid> {grid::getVertexCentre(p)}
		, nvecs_ {grid::numVertices(p)}
		, vecs_ {grid::getVertexVectors(p)}
	{}

	iter begin()
	{
		return iter {this->pt_, 0, vecs_};
	}
	iter end()
	{
		return iter {this->pt_, nvecs_, vecs_};
	}

	size_t nvecs_;
	const point<int8_t> *vecs_;
};
