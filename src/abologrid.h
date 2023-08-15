#pragma once

#include <cstdint>
#include <iterator>

#include "geom.h"

template<typename coord>
class AboloGrid
{
public:
    using coord_t = coord;
    using point_t = point<coord>;
    using xform_t = xform<coord>;

    enum TileType {
		INVALID = -1,
        TRIANGLE_UR = 0, 
		TRIANGLE_UL = 1,
		TRIANGLE_LL = 2,
		TRIANGLE_LR = 3,
    };

public:
    inline static size_t num_tile_types = 4; 
    inline static size_t num_tile_shapes = 1;

	inline static TileType getTileType( const point_t& p )
	{
		if( p.x_ % 2 == 0 ) {
			if( p.y_ % 2 == 0 ) {
				return TRIANGLE_UR;
			} else {
				return TRIANGLE_LR;
			}
		} else {
			if( p.y_ % 2 == 0 ) {
				return TRIANGLE_UL;
			} else {
				return TRIANGLE_LL;
			}
		}
	}

    inline static point_t getOrigin( const point_t& p ) 
    {
		return origins[ (size_t)getTileType( p ) ];
    }

    static size_t numNeighbours( const point_t& p )
    {
        return 14;
    }

    static const point<int8_t> *getNeighbourVectors( const point_t& p )
    {
        return all_neighbours[getTileType(p)];
    }

    static size_t numEdgeNeighbours( const point_t& p )
    {
        return 3;
    }

    static const point<int8_t> *getEdgeNeighbourVectors( const point_t& p )
    {
        return edge_neighbours[getTileType(p)];
    }

    static bool translatable( const point_t& p, const point_t& q )
    {
        return getTileType( p ) == getTileType( q );
    }

    static const size_t num_orientations;
    static const xform<int8_t> orientations[8];

    static const point<int8_t> all_neighbours[4][14];
    static const point<int8_t> edge_neighbours[4][3];
    static const point<int8_t> origins[4];

    static const std::vector<point<int8_t>> vertices[4];

    static std::vector<point_t> getCellVertices( const point_t& p )
    {
        const auto &vertexVecs = vertices[getTileType(p)];
        std::vector<point_t> ans(vertexVecs.size());
        point_t pTrans = p + p;
        for (size_t i = 0; i < vertexVecs.size(); ++i)
            ans[i] = pTrans + vertexVecs[i];
        return ans;
    }

    static point<double> vertexToGrid( const point_t& pt )
	{
        return {pt.x_ / 2.0, pt.y_ / 2.0};
    }

    static point<double> gridToPage( const point<double>& pt )
	{
        return pt;
    }
};

template<typename coord>
const point<int8_t> AboloGrid<coord>::all_neighbours[4][14] = {
        {
                {1, 0},
                {0, 1},
                {-1, -1},
                {2, -1},
                {2, -2},
                {1, -3},
                {0, -3},
                {-1, -2},
                {-2, -1},
                {-3, 0},
                {-3, 1},
                {-2, 2},
                {-1, 2},
                {1, 1}
        },
        {
                {-1, 0},
                {0, 1},
                {1, -1},
                {1, 2},
                {2, 2},
                {3, 1},
                {3, 0},
                {2, -1},
                {1, -2},
                {0, -3},
                {-1, -3},
                {-2, -2},
                {-2, -1},
                {-1, 1}
        },
        {
                {-1, 0},
                {0, -1},
                {1, 1},
                {-2, 1},
                {-2, 2},
                {-1, 3},
                {0, 3},
                {1, 2},
                {2, 1},
                {3, 0},
                {3, -1},
                {2, -2},
                {1, -2},
                {-1, -1}
        },
        {
                {1, 0},
                {0, -1},
                {-1, 1},
                {-1, -2},
                {-2, -2},
                {-3, -1},
                {-3, 0},
                {-2, 1},
                {-1, 2},
                {0, 3},
                {1, 3},
                {2, 2},
                {2, 1},
                {1, -1}
        }
};

template<typename coord>
const point<int8_t> AboloGrid<coord>::edge_neighbours[4][3] = {
        {
                {1, 0},
                {0, 1},
                {-1, -1}
        },
        {
                {-1, 0},
                {0, 1},
                {1, -1}
        },
        {
                {-1, 0},
                {0, -1},
                {1, 1}
        },
        {
                {1, 0},
                {0, -1},
                {-1, 1}
        }
};

template<typename coord>
const point<int8_t> AboloGrid<coord>::origins[4] = {
        {0, 0},
        {1, 0},
        {1, 1},
        {0, 1}
};

template<typename coord>
const size_t AboloGrid<coord>::num_orientations = 8;

template<typename coord>
const xform<int8_t> AboloGrid<coord>::orientations[8] = {
        { 1, 0, 0, 0, 1, 0 },
        { 0, -1, 1, 1, 0, 0 },
        { -1, 0, 1, 0, -1, 1 },
        { 0, 1, 0, -1, 0, 1 },

        { -1, 0, 1, 0, 1, 0 },
        { 0, -1, 1, -1, 0, 1 },
        { 1, 0, 0, 0, -1, 1 },
        { 0, 1, 0, 1, 0, 0 } };

template<typename coord>
const std::vector<point<int8_t>> AboloGrid<coord>::vertices[4] = {
        {
                {1, 1}, {1, -3}, {-3, 1}
        },
        {
                {-1, 1}, {3, 1}, {-1, -3}
        },
        {
                {-1, -1}, {-1, 3}, {3, -1}
        },
        {
                {1, -1}, {-3, -1}, {1, 3}
        }
};
