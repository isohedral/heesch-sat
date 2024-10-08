#pragma once

#include <cstdint>
#include <iterator>

#include "geom.h"

template<typename coord>
class DrafterGrid
{
public:
    using coord_t = coord;
    using point_t = point<coord>;
    using xform_t = xform<coord>;
    using edge_t = std::pair<point_t, point_t>;

    enum TileType {
        // Triangles in CCW order starting from the +x-axis
        INVALID = -1,
        TRIANGLE_0 = 0,
		TRIANGLE_1 = 1,
		TRIANGLE_2 = 2,
		TRIANGLE_3 = 3,
        TRIANGLE_4 = 4,
		TRIANGLE_5 = 5,
		TRIANGLE_6 = 6,
		TRIANGLE_7 = 7,
		TRIANGLE_8 = 8, 
		TRIANGLE_9 = 9,
		TRIANGLE_10 = 10,
		TRIANGLE_11 = 11
    };

	enum TileShape {
		TRIANGLE_SHAPE = 0
	};

public:
	inline static GridType grid_type = DRAFTER;

    inline static size_t num_tile_types = 12; 
    inline static size_t num_tile_shapes = 1;

    inline static TileType getTileType( const point_t& p )
    {
		coord_t mx = ((p.x_ % 7) + 7) % 7;
		coord_t my = ((p.y_ % 7) + 7) % 7;

		static const TileType types[] = {
			INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
			INVALID, INVALID, TRIANGLE_0, INVALID, TRIANGLE_5, INVALID,
			INVALID, INVALID, TRIANGLE_1, INVALID, INVALID, TRIANGLE_4,
			INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, INVALID,
			TRIANGLE_3, TRIANGLE_2, INVALID, TRIANGLE_8, TRIANGLE_9, INVALID,
			INVALID, INVALID, INVALID, INVALID, INVALID, INVALID, TRIANGLE_10,
			INVALID, INVALID, TRIANGLE_7, INVALID, INVALID, INVALID,
			TRIANGLE_11, INVALID, TRIANGLE_6, INVALID
		};

		return types[ my*7 + mx ];
    }
	inline static TileShape getTileShape( const point_t& p )
	{
		return TRIANGLE_SHAPE;
	}

    inline static point_t getOrigin( const point_t& p ) 
    {
		return origins[ (size_t)getTileType( p ) ];
    }

    static size_t numNeighbours( const point_t& p )
    {
        return 16;
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
        TileType t1 = getTileType(p), t2 = getTileType(q);
        return t1 == t2 && t1 != INVALID && t2 != INVALID;
    }

    static const size_t num_orientations;
    static const xform<int8_t> orientations[12];

    static const point<int8_t> all_neighbours[12][16];
    static const point<int8_t> edge_neighbours[12][3];
    static const point<int8_t> origins[12];

    static const std::vector<point<int8_t>> vertices[12];
    static const std::vector<point<int8_t>> boundaryWordDirections;

    static std::vector<point_t> getCellVertices( const point_t& p )
    {
		static const point_t los[12] = {
			{ -2, -1 }, { -1, -2 },
			{ 1, -3 }, { 2, -3 },
			{ 3, -2 }, { 3, -1 },
			{ 2, 1 }, { 1, 2 },
			{ -1, 3 }, { -2, 3 },
			{ -3, 2 }, { -3, 1 } };

		auto ttype = getTileType( p );
        const auto &vertexVecs = vertices[ttype];
        std::vector<point_t> ans(vertexVecs.size());
        point_t pTrans = p + los[ttype];
		pTrans = point_t { 
			(coord_t)(pTrans.x_ * 12 / 7), (coord_t)(pTrans.y_ * 12 / 7) };

        for (size_t i = 0; i < vertexVecs.size(); ++i)
            ans[i] = pTrans + vertexVecs[i];
        return ans;
    }

    static point<double> vertexToGrid( const point_t& pt )
	{
        return {pt.x_ / 6.0 * 3.5, (double)pt.y_ / 6.0 * 3.5};
    }

    static point<double> gridToPage( const point<double>& pt )
	{
        const double hr3 = 0.5 * 1.73205080756887729353;
		return { pt.x_ + 0.5*pt.y_, hr3 * pt.y_ };
    }
};

template<typename coord>
const point<int8_t> DrafterGrid<coord>::all_neighbours[12][16] = {
        {
                { 2, 0},
                { -1, 1},
                { 1, -2},
                {-3, 2},
                {-4, 2},
                {-5, 1},
                {-5, 0},
                {-4, -2},
                {-3, -3},
                {-1, -4},
                {0, -4},
                {1, -3},
                {2, 1},
                {0, 3},
                {-1, 3},
                {3, -2}
        },
        {
                { 1, -1},
                { 0, 2},
                { -2, 1},
                {-3, 1},
                {-4, 0},
                {-4, -1},
                {-3, -3},
                {-2, -4},
                {0, -5},
                {1, -5},
                {2, -4},
                {2, -3},
                {1, 2},
                {3, 0},
                {3, -1},
                {-2, 3}
        },
        {
                { 2, -1},
                { 0, 2},
                { -1, 0},
                {-2, -1},
                {-2, -2},
                {-1, -4},
                {0, -5},
                {2, -6},
                {3, -6},
                {4, -5},
                {4, -4},
                {3, -2},
                {-3, 2},
                {-3, 3},
                {-1, 3},
                {2, 1}
        },
        {
                { 1, 0},
                { -2, 2},
                { -1, -1},
                {-1, -2},
                {0, -4},
                {1, -5},
                {3, -6},
                {4, -6},
                {5, -5},
                {5, -4},
                {4, -2},
                {3, -1},
                {-2, 3},
                {0, 3},
                {1, 2},
                {-3, 1}
        },
        {
                { 1, 1},
                { -2, 2},
                { 0, -1},
                {1, -3},
                {2, -4},
                {4, -5},
                {5, -5},
                {6, -4},
                {6, -3},
                {5, -1},
                {4, 0},
                {2, 1},
                {-2, -1},
                {-3, 0},
                {-3, 2},
                {-1, 3}
        },
        {
                { 0, 1},
                { -2, 0},
                { 1, -2},
                {2, -3},
                {4, -4},
                {5, -4},
                {6, -3},
                {6, -2},
                {5, 0},
                {4, 1},
                {2, 2},
                {1, 2},
                {-3, 1},
                {-3, 3},
                {-2, 3},
                {-1, -2}
        },
        {
                { -1, 2},
                { 1, -1},
                { -2, 0},
                {3, -2},
                {4, -2},
                {5, -1},
                {5, 0},
                {4, 2},
                {3, 3},
                {1, 4},
                {0, 4},
                {-1, 3},
                {-2, -1},
                {0, -3},
                {1, -3},
                {-3, 2}
        },
        {
                { 2, -1},
                { 0, -2},
                { -1, 1},
                {3, -1},
                {4, 0},
                {4, 1},
                {3, 3},
                {2, 4},
                {0, 5},
                {-1, 5},
                {-2, 4},
                {-2, 3},
                {-3, 0},
                {-1, -2},
                {-3, 1},
                {2, -3}
        },
        {
                { 1, 0},
                { -2, 1},
                { 0, -2},
                {2, 1},
                {2, 2},
                {1, 4},
                {0, 5},
                {-2, 6},
                {-3, 6},
                {-4, 5},
                {-4, 4},
                {-3, 2},
                {1, -3},
                {3, -3},
                {3, -2},
                {-2, -1}
        },
        {
                { 1, 1},
                { -1, 0},
                { 2, -2},
                {1, 2},
                {0, 4},
                {-1, 5},
                {-3, 6},
                {-4, 6},
                {-5, 5},
                {-5, 4},
                {-4, 2},
                {-3, 1},
                {-1, -2},
                {0, -3},
                {2, -3},
                {3, -1}
        },
        {
                { 0, 1},
                { 2, -2},
                { -1, -1},
                {-1, 3},
                {-2, 4},
                {-4, 5},
                {-5, 5},
                {-6, 4},
                {-6, 3},
                {-5, 1},
                {-4, 0},
                {-2, -1},
                {2, 1},
                {3, 0},
                {3, -2},
                {1, -3}
        },
        {
                { 2, 0},
                { -1, 2},
                { 0, -1},
                {-2, 3},
                {-4, 4},
                {-5, 4},
                {-6, 3},
                {-6, 2},
                {-5, 0},
                {-4, -1},
                {-2, -2},
                {-1, -2},
                {3, -1},
                {3, -3},
                {2, -3},
                {1, 2}
        }
};

template<typename coord>
const point<int8_t> DrafterGrid<coord>::edge_neighbours[12][3] = {
        {
                { 2, 0},
                { -1, 1},
                { 1, -2},
        },
        {
                { 1, -1},
                { 0, 2},
                { -2, 1},
        },
        {
                { 2, -1},
                { 0, 2},
                { -1, 0},
        },
        {
                { 1, 0},
                { -2, 2},
                { -1, -1},
        },
        {
                { 1, 1},
                { -2, 2},
                { 0, -1},
        },
        {
                { 0, 1},
                { -2, 0},
                { 1, -2},
        },
        {
                { -1, 2},
                { 1, -1},
                { -2, 0},
        },
        {
                { 2, -1},
                { 0, -2},
                { -1, 1},
        },
        {
                { 1, 0},
                { -2, 1},
                { 0, -2},
        },
        {
                { 1, 1},
                { -1, 0},
                { 2, -2},
        },
        {
                { 0, 1},
                { 2, -2},
                { -1, -1},
        },
        {
                { 2, 0},
                { -1, 2},
                { 0, -1},
        }
};

template<typename coord>
const point<int8_t> DrafterGrid<coord>::origins[12] = {
        {2, 1},
        {1, 2},
        {6, 3},
        {5, 3},
        {4, 2},
        {4, 1},
        {5, 6},
        {6, 5},
        {1, 4},
        {2, 4},
        {3, 5},
        {3, 6}
};

template<typename coord>
const size_t DrafterGrid<coord>::num_orientations = 12;

template<typename coord>
const xform<int8_t> DrafterGrid<coord>::orientations[12] = {
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

template<typename coord>
const std::vector<point<int8_t>> DrafterGrid<coord>::vertices[12] = {
        { // {2,1}
                {0, 0}, {6, 0}, {4, 4}
        },
        { // {1,2}
                {4, 4}, {0, 6}, {0, 0}
        },
        { // {6,3}
                {0, 0}, {0,6}, {-4,8}
        },
        { // {5,3}
                {0, 0}, {-4, 8}, {-6,6}
        },
        { // {4,2}
                {0, 0}, {-6, 6}, {-8, 4}
        },
        { // {4,1}
                {0, 0}, {-8, 4}, {-6, 0}
        },
        { // {5,6}
                {0, 0}, {-6, 0}, {-4, -4}
        },
        { // {6,5}
                {0, 0}, {-4, -4}, {0, -6}
        },
        { // {1,4}
                {0, 0}, {0, -6}, {4, -8}
        },
        { // {2,4}
                {0, 0}, {4, -8}, {6, -6}
        },
        { // {3,5}
                {0, 0}, {6, -6}, {8, -4}
        },
        { // {3,6}
                {0, 0}, {8, -4}, {6, 0}
        }
};
