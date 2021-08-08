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
    using edge_t = std::pair<point_t, point_t>;

    enum TileType {
        TRIANGLE_UR, TRIANGLE_UL, TRIANGLE_LL, TRIANGLE_LR,
        INVALID
    };

    enum TileShape {
        TRIANGLE_SHAPE
    };

public:
    static size_t numTileTypes() { return 4; }
    static size_t numTileShapes() { return 1; }

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
        TileType t1 = getTileType(p), t2 = getTileType(q);
        return t1 == t2 && t1 != INVALID && t2 != INVALID;
    }

    static const size_t num_orientations;
    static const xform<int8_t> orientations[8];

    static const point<int8_t> all_neighbours[4][14];
    static const point<int8_t> edge_neighbours[4][3];
    static const point<int8_t> origins[4];

    static const std::vector<point<int8_t>> vertices[4];
    static const std::vector<point<int8_t>> boundaryWordDirections;

    static const std::vector<std::vector<point<int8_t>>> mates[4];

    static point_t getTileTypeOrigin(TileType t)
    {
        return origins[t];
    }

    static TileType getTileType( const point_t& p )
    {
        point_t p2 = p;
        if ((p2.y_/2) % 2 == 0) p2.x_ -= 2;
        p2.x_ = (p2.x_%2 + 2) % 2;
        p2.y_ = (p2.y_%2 + 2) % 2;
        return (TileType) (std::find(
                origins, origins + numTileTypes() + 1,
                p2)
                - origins);
    }

    static TileShape getTileShape( const TileType t )
    {
        return TRIANGLE_SHAPE;
    }

    static TileShape getTileShape( const point_t& p )
    {
        return getTileShape(getTileType(p));
    }

    static std::vector<edge_t> getTileEdges( const point_t& p )
    {
        std::vector<edge_t> edges;
        auto vertices = getTileVertices(p);
        for (size_t i = 0; i < vertices.size(); ++i)
            edges.emplace_back(vertices[i], vertices[(i+1) % vertices.size()]);
        return edges;
    }

    static int8_t getBoundaryWordDirection( const point_t &dir ) {
        return std::find(boundaryWordDirections.begin(), boundaryWordDirections.end(), dir) - boundaryWordDirections.begin();
    }

    static size_t numRotations() { return 4; }
    static int8_t rotateDirection( int8_t dir ) { return (dir + 2) % boundaryWordDirections.size(); }
    static int8_t reflectDirection( int8_t dir ) { return (boundaryWordDirections.size() - dir) % boundaryWordDirections.size(); }

    static bool hasMates() { return true; }

    static std::vector<std::vector<point_t>> getMatesList(const point_t &p) {
        std::vector<std::vector<point_t>> matesList;
        for (auto &v : mates[getTileType(p)]) {
            matesList.emplace_back();
            for (auto &dir : v) matesList.back().push_back(dir + p);
        }
        return matesList;
    }

private:
    static std::vector<point_t> getTileVertices( const point_t& p )
    {
        const auto &vertexVecs = vertices[getTileType(p)];
        std::vector<point_t> ans(vertexVecs.size());
        point_t pTrans = p + p;
        for (size_t i = 0; i < vertexVecs.size(); ++i)
            ans[i] = vertexVecs[i] + pTrans;
        return ans;
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

template<typename coord>
const std::vector<point<int8_t>> AboloGrid<coord>::boundaryWordDirections = {
        {4, 0}, {4, -4}, {0, -4}, {-4, -4}, {-4, 0}, {-4, 4}, {0, 4}, {4, 4}};

template<typename coord>
const std::vector<std::vector<point<int8_t>>> AboloGrid<coord>::mates[4] = {
        {
                {{1, 0}},
                {{0, 1}}
        },
        {
                {{-1, 0}},
                {{0, 1}}
        },
        {
                {{-1, 0}},
                {{0, -1}},
        },
        {
                {{1, 0}},
                {{0, -1}}
        }
};