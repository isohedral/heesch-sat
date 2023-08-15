#pragma once

#include <cstdint>
#include <iterator>

#include "geom.h"

template<typename coord>
class TriHexGrid
{
public:
	using coord_t = coord;
	using point_t = point<coord>;
	using xform_t = xform<coord>;
	using edge_t = std::pair<point_t, point_t>;

    enum TileType {
		INVALID = -1,
        HEXAGON = 0,
		TRIANGLE_RIGHT = 1,
		TRIANGLE_LEFT = 2
    };

public:
    inline static size_t num_tile_types = 3; 
    inline static size_t num_tile_shapes = 2;

    static TileType getTileType( const point_t& p )
    {
        return (TileType) (((p.x_ - p.y_) % 3 + 3) % 3);
    }

    inline static point_t getOrigin( const point_t& p ) 
    {
		return origins[ (size_t)getTileType( p ) ];
    }

	static size_t numNeighbours( const point_t& p )
	{
		return getTileType(p) == HEXAGON ? 12 : 6;
	}

	static const point<int8_t> *getNeighbourVectors( const point_t& p )
	{
        return all_neighbours[getTileType(p)];
	}

	static size_t numEdgeNeighbours( const point_t& p )
	{
        return getTileType(p) == HEXAGON ? 6 : 3;
	}

	static const point<int8_t> *getEdgeNeighbourVectors( const point_t& p )
	{
        return edge_neighbours[getTileType(p)];
	}

	static bool translatable( const point_t& p, const point_t& q )
	{
		return getTileType(p) == getTileType(q);
	}

	static const size_t num_orientations;
	static const xform<int8_t> orientations[12];
	
	static const point<int8_t> all_neighbours[3][12];
	static const point<int8_t> edge_neighbours[3][6];
    static const point<int8_t> origins[3];

	static const std::vector<point<int8_t>> vertices[3];

    static std::vector<point_t> getCellVertices( const point_t& p )
    {
        const auto &vertexVecs = vertices[getTileType(p)];
        std::vector<point_t> ans(vertexVecs.size());
        for (size_t i = 0; i < vertexVecs.size(); ++i)
            ans[i] = p + p + vertexVecs[i];
        return ans;
    }

    static point<double> vertexToGrid( const point_t& pt )
	{
        return {pt.x_ / 2.0, pt.y_ / 2.0};
    }

    static point<double> gridToPage( const point<double>& pt )
	{
        const double sqrt3 = 1.73205080756887729353;
		return { pt.x_ + 0.5*pt.y_, 0.5 * sqrt3 * pt.y_ };
    }
};

template<typename coord>
const point<int8_t> TriHexGrid<coord>::all_neighbours[3][12] = {
    {
        { 0, -1 },
        { 1, -1 },
        { -1, 0 },
        { 1, 0 },
        { -1, 1 },
        { 0, 1 },
        {-2, 1},
        {-1, 2},
        {1, 1},
        {2, -1},
        {1, -2},
        {-1, -1}
    },
    {
        {1, 0},
        {-1, 1},
        {0, -1},
        {0, 1},
        {1, -1},
        {-1, 0}
    },
    {
        {1, 0},
        {-1, 1},
        {0, -1},
        {0, 1},
        {1, -1},
        {-1, 0}
    }
};

template<typename coord>
const point<int8_t> TriHexGrid<coord>::edge_neighbours[3][6] = {
    {
        { 0, -1 },
        { 0, 1 },
        { 1, 0 },
        { -1, 0 },
        { 1, -1 },
        { -1, 1 }
    },
    {
        {0, 1},
        {1, -1},
        {-1, 0}
    },
    {
        {1, 0},
        {-1, 1},
        {0, -1}
    }
};

template<typename coord>
const point<int8_t> TriHexGrid<coord>::origins[3] = {
    { 0, 0 },
    { 1, 0 },
    { 2, 0 }
};

template<typename coord>
const size_t TriHexGrid<coord>::num_orientations = 12;

template<typename coord>
const xform<int8_t> TriHexGrid<coord>::orientations[12] = {
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
const std::vector<point<int8_t>> TriHexGrid<coord>::vertices[3] = {
    {
        {-1, -1}, {-2, 1}, {-1, 2}, {1, 1}, {2, -1}, {1, -2}
    },
    {
        {-1, 1}, {1, 0}, {0, -1}
    },
    {
        {-1, 0}, {0, 1}, {1, -1}
    }
};
