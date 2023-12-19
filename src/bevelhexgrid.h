#pragma once

#include <cstdint>
#include <iterator>

#include "geom.h"

// The Archimedean tiling [4.6.12]
template<typename coord>
class BevelHexGrid
{
public:
	using coord_t = coord;
	using point_t = point<coord>;
	using xform_t = xform<coord>;
	using edge_t = std::pair<point_t, point_t>;

    enum TileType {
		INVALID = -1,
        SQUARE_E = 0,
        SQUARE_NE = 1,
        SQUARE_NW = 2,
        HEXAGON_A = 3,
        HEXAGON_Y = 4,
		DODECAGON = 5
    };

    enum TileShape {
		INVALID_SHAPE = -1,
        SQUARE_SHAPE = 0,
        HEXAGON_SHAPE = 1,
		DODECAGON_SHAPE = 2
    };

public:
	inline static GridType grid_type = BEVELHEX;

    inline static size_t num_tile_types = 6; 
    inline static size_t num_tile_shapes = 3;

    inline static TileType getTileType( const point_t& p )
    {
		size_t cx = ((p.x_ % 6) + 6) % 6;
		size_t cy = ((p.y_ % 6) + 6) % 6;

		static const TileType types[] = {
			DODECAGON, INVALID, INVALID, SQUARE_E, INVALID, INVALID,
			INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
			INVALID, INVALID, HEXAGON_Y, INVALID, INVALID, INVALID,
			SQUARE_NE, INVALID, INVALID, SQUARE_NW, INVALID, INVALID,
			INVALID, INVALID, INVALID, INVALID, HEXAGON_A, INVALID,
			INVALID, INVALID, INVALID, INVALID, INVALID, INVALID
		};

		TileType ret = types[ cy * 6 + cx ];
		if( ret == INVALID ) {
			std::cerr << "Queried illegal bevelhex position" << std::endl;
		}
		return ret;
    }
	inline static TileShape getTileShape( const point_t& p )
	{
		switch( getTileType( p ) ) {
			case SQUARE_E: 
			case SQUARE_NE:
			case SQUARE_NW:
				return SQUARE_SHAPE;
			case HEXAGON_A:
			case HEXAGON_Y:
				return HEXAGON_SHAPE;
			case DODECAGON:
				return DODECAGON_SHAPE;
			default:
				return INVALID_SHAPE;
		}
	}

    inline static point_t getOrigin( const point_t& p ) 
    {
		return origins[ (size_t)getTileType( p ) ];
    }

	static size_t numNeighbours( const point_t& p )
	{
		static const size_t num_sides[] = { 4, 4, 4, 6, 6, 12 };
		return num_sides[(size_t)getTileType( p )];
	}

	static const point<int8_t> *getNeighbourVectors( const point_t& p )
	{
        return neighbours[(size_t)getTileType( p )];
	}

	static size_t numEdgeNeighbours( const point_t& p )
	{
		return numNeighbours( p );
	}

	static const point<int8_t> *getEdgeNeighbourVectors( const point_t& p )
	{
		return getNeighbourVectors( p );
	}

	static bool translatable( const point_t& p, const point_t& q )
	{
		return getTileType(p) == getTileType(q);
	}

	static const point_t origins[6];

	static const size_t num_orientations = 12;
	static const xform<int8_t> orientations[12];
	
	static const point<int8_t> neighbours[6][12];
	static const point_t cell_vertices[6][12];

    static std::vector<point_t> getCellVertices( const point_t& p )
    {
		point_t o = getOrigin( p );
		size_t nn = numNeighbours( p );
		const auto *cvs = cell_vertices[(size_t)getTileType( p )];

		std::vector<point_t> ret;
		ret.reserve( nn );

		for( size_t idx = 0; idx < nn; ++idx ) {
			ret.emplace_back(
				p.x_ - o.x_ + cvs[idx].x_, p.y_ - o.y_ + cvs[idx].y_ );
		}
		return ret;
    }

    static point<double> vertexToGrid( const point_t& pt ) 
	{
		return { (double)pt.x_, (double)pt.y_ };
    }

    static point<double> gridToPage( const point<double>& pt) {
        const double sqrt3 = 1.73205080756887729353;
		return { pt.x_ + 0.5*pt.y_, 0.5 * sqrt3 * pt.y_ };
    }
};

template<typename coord>
const point<coord> BevelHexGrid<coord>::origins[6] = {
	{ 3, 0 }, // SQUARE_E
    { 0, 3 }, // SQUARE_NE
 	{ 3, 3 }, // SQUARE_NW
	{ 4, 4 }, // HEXAGON_A
	{ 2, 2 }, // HEXAGON_Y
	{ 0, 0 } // DODECAGON
};

template<typename coord>
const point<int8_t> BevelHexGrid<coord>::neighbours[6][12] = {
	{ // SQUARE_E
		{ 3, 0 }, { -1, 2 }, { -3, 0 }, { 1, -2 }
	},
	{ // SQUARE_NE
		{ 2, -1 }, { 0, 3 }, { -2, 1 }, { 0, -3 }
	},
	{ // SQUARE_NW
		{ 1, 1 }, { -3, 3 }, { -1, -1 }, { 3, -3 }
	},
	{ // HEXAGON_A
		{ 2, -1 }, { 2, 2 }, { -1, 2 }, { -4, 2 }, { -1, -1 }, { 2, -4 }
	},
	{ // HEXAGON_Y
		{ 1, 1 }, { -2, 4 }, { -2, 1 }, { -2, -2 }, { 1, -2 }, { 4, -2 }
	},
	{ // DODECAGON
		{ 3, 0 }, { 2, 2 }, { 0, 3 }, { -2, 4 }, { -3, 3 }, { -4, 2 },
		{ -3, 0 }, { -2, -2 }, { 0, -3 }, { 2, -4 }, { 3, -3 }, { 4, -2 } 
	}
};

template<typename coord>
const point<coord> BevelHexGrid<coord>::cell_vertices[6][12] = {
	{ // SQUARE_E
		{ 3, -1 }, { 4, -1 }, { 3, 1 }, { 2, 1 }
	},
	{ // SQUARE_NE
		{ 1, 2 }, { 1, 3 }, { -1, 4 }, { -1, 3 }
	},
	{ // SQUARE_NW
		{ 3, 2 }, { 4, 3 }, { 3, 4 }, { 2, 3 }
	},
	{ // HEXAGON_A
		{ 5, 4 }, { 4, 5 }, { 3, 5 }, { 3, 4 }, { 4, 3 }, { 5, 3 }
	},
	{ // HEXAGON_Y
		{ 2, 1 }, { 3, 1 }, { 3, 2 }, { 2, 3 }, { 1, 3 }, { 1, 2 }
	},
	{ // DODECAGON
		{ 2, 1 }, { 1, 2 }, { -1, 3 }, { -2, 3 }, { -3, 2 }, { -3, 1 },
		{ -2, -1 }, { -1, -2 }, { 1, -3 }, { 2, -3 }, { 3, -2 }, { 3, -1 } 
	}
};

template<typename coord>
const xform<int8_t> BevelHexGrid<coord>::orientations[12] = {
        { 1, 0, 0,     0, 1, 0 },
        { 0, -1, 0,    1, 1, 0 },
        { -1, -1, 0,   1, 0, 0 },
        { -1, 0, 0,    0, -1, 0 },
        { 0, 1, 0,     -1, -1, 0 },
        { 1, 1, 0,     -1, 0, 0 },

        { 0, 1, 0,     1, 0, 0 },
        { -1, 0, 0,    1, 1, 0 },
        { -1, -1, 0,   0, 1, 0 },
        { 0, -1, 0,    -1, 0, 0 },
        { 1, 0, 0,     -1, -1, 0 },
        { 1, 1, 0,     0, -1, 0 } };
