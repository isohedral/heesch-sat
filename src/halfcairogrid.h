#pragma once

#include <cstdint>
#include <iterator>

#include "geom.h"

template<typename coord>
class HalfCairoGrid
{
public:
	using coord_t = coord;
	using point_t = point<coord>;
	using xform_t = xform<coord>;
	using edge_t = std::pair<point_t, point_t>;

	// CCW from East within one square
	enum TileType {
		INVALID = -1,
		TRIANGLE_E = 0,
		KITE_NE = 1,
		TRIANGLE_N = 2,
		KITE_NW = 3,
		TRIANGLE_W = 4,
		KITE_SW = 5,
		TRIANGLE_S = 6,
		KITE_SE = 7
	};

public:
    inline static size_t numTileTypes = 8;
    inline static size_t numTileShapes = 2;
	inline static TileType getTileType( const point_t& p )
	{
		int xm = ((p.x_%3) + 3) % 3;
		int ym = ((p.y_%3) + 3) % 3;
		const TileType types[] = {
			INVALID, TRIANGLE_E, TRIANGLE_W, TRIANGLE_N,
			KITE_NE, KITE_NW, TRIANGLE_S, KITE_SE, KITE_SW };

		return types[ (ym*3) + xm ];
	}

    inline static point_t getOrigin( const point_t& p ) 
    {
		return p - getCellHub( p );
	}

	static size_t numNeighbours( const point_t& p )
	{
		return (getTileType(p) % 2 == 0) ? 10 : 12;
	}

	static const point<int8_t> *getNeighbourVectors( const point_t& p )
	{
        return neighbour_vectors[ getTileType( p ) ];
	}

	static size_t numEdgeNeighbours( const point_t& p )
	{
		return (getTileType(p) % 2 == 0) ? 3 : 4;
	}

	static const point<int8_t> *getEdgeNeighbourVectors( const point_t& p )
	{
        return edge_neighbour_vectors[ getTileType( p ) ];
	}

	static bool translatable( const point_t& p, const point_t& q )
	{
		return getTileType(p) == getTileType(q);
	}

	static const point_t origins[8];

	inline static size_t num_orientations = 8;
	static const xform<int8_t> orientations[8];
	
	static const point<int8_t> neighbour_vectors[8][12];
	static const point<int8_t> edge_neighbour_vectors[8][4];

	static point_t getCellHub( const point_t& p )
	{
		int xc = (p.x_>=0) ? ((p.x_+1)/3*4) : ((p.x_-1)/3*4);
		int yc = (p.y_>=0) ? ((p.y_+1)/3*4) : ((p.y_-1)/3*4);
		return point_t( xc, yc );
	}

    static std::vector<point_t> getCellVertices( const point_t& p )
    {
		static const point<int8_t> offs[8] = { 
			{ 2, -1 }, { 2, 1 }, { 1, 2 }, { -1, 2 }, 
			{ -2, 1 }, { -2, -1 }, { -1, -2 }, { 1, -2 } };
		static const point<int8_t> corners[4] = { 
			{ 2, 2 }, { -2, 2 }, { -2, -2 }, { 2, -2 } };

		int ttype = (int)getTileType( p );
		point_t pc = getCellHub( p );
		std::vector<point_t> ret = { pc, pc + offs[ttype] };;
		if( (ttype%2) == 1 ) {
			ret.push_back( pc + corners[ttype/2] );
		}
		ret.push_back( pc + offs[(ttype+1)%8] );
		return ret;
    }

    static point<double> vertexToGrid( const point_t& pt )
	{
		// bool xm4 = (pt.x_%4) == 0;
		// bool ym4 = (pt.y_%4) == 0;

		return point<double> { 0.75 * pt.x_, 0.75 * pt.y_ };
		/*
		if( xm4 && ym4 ) {
			return point<double> { 0.75 * pt.x_, 0.75 * pt.y_ };
		} else if( xm4 ) {
			return point<double>
		}
		*/
    }

    static point<double> gridToPage( const point<double>& pt) {
        return pt;
    }
};

template<typename coord>
const point<coord> HalfCairoGrid<coord>::origins[8] = {
	{ 1, 0 }, { 1, 1 }, { 0, 1 }, 
	{ -1, 1 }, { -1, 0 }, { -1, -1 }, 
	{ 0, -1 }, { 1, -1 }
};

template<typename coord>
const point<int8_t> HalfCairoGrid<coord>::neighbour_vectors[8][12] = {
	{ 
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -2, 1 }, { -2, 0 },
		{ -2, -1 }, { -1, -1 }, { 0, -1 }, { 1, -1 }
	}, 
	{ 
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -2, 0 },
		{ -2, -1 }, { -2, -2 }, { -1, -2 }, { 0, -2 }, { 0, -1 }, { 1, -1 }
	}, 
	{ 
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 },
		{ -1, -2 }, { 0, -2 }, { 1, -2 }, { 1, -1 }
	}, 
	{ 
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 },
		{ 0, -1 }, { 0, -2 }, { 1, -2 }, { 2, -2 }, { 2, -1 }, { 2, 0 }
	}, 
	{ 
		{ 2, 0 }, { 2, 1 }, { 1, 1 }, { 0, 1 }, { -1, 1 }, { -1, 0 },
		{ -1, -1 }, { 0, -1 }, { 1, -1 }, { 2, -1 }
	}, 
	{ 
		{ 1, 0 }, { 2, 0 }, { 2, 1 }, { 2, 2 }, { 1, 2 }, { 0, 2 }, 
		{ 0, 1 }, { -1, 1 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 }
	}, 
	{ 
		{ 1, 0 }, { 1, 1 }, { 1, 2 }, { 0, 2 }, { -1, 2 }, { -1, 1 },
		{ -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 }
	}, 
	{ 
		{ 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 2 }, { -1, 2 }, { -2, 2 }, 
		{ -2, 1 }, { -2, 0 }, { -1, 0 }, { -1, -1 }, { 0, -1 }, { 1, -1 }
	}
};

template<typename coord>
const point<int8_t> HalfCairoGrid<coord>::edge_neighbour_vectors[8][4] = {
	{ { 1, 0 }, { 0, 1 }, { 0, -1 } }, 
	{ { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } },
	{ { 1, 0 }, { 0, 1 }, { -1, 0 } }, 
	{ { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } },
	{ { 0, 1 }, { -1, 0 }, { 0, -1 } }, 
	{ { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } },
	{ { 1, 0 }, { -1, 0 }, { 0, -1 } }, 
	{ { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 } }
};

template<typename coord>
const xform<int8_t> HalfCairoGrid<coord>::orientations[8] = {
	{ 1, 0, 0, 0, 1, 0 }, { 0, -1, 0, 1, 0, 0 }, 
	{ -1, 0, 0, 0, -1, 0 }, { 0, 1, 0, -1, 0, 0 },
	{ -1, 0, 0, 0, 1, 0 }, { 0, -1, 0, -1, 0, 0 },
	{ 1, 0, 0, 0, -1, 0 }, { 0, 1, 0, 1, 0, 0 } };
