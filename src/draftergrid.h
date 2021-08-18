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
        TRIANGLE_0, TRIANGLE_1, TRIANGLE_2, TRIANGLE_3,
        TRIANGLE_4, TRIANGLE_5, TRIANGLE_6, TRIANGLE_7,
        TRIANGLE_8, TRIANGLE_9, TRIANGLE_10, TRIANGLE_11,
        INVALID
    };

    enum TileShape {
        TRIANGLE_SHAPE
    };

public:
    static size_t numTileTypes() { return 12; }
    static size_t numTileShapes() { return 1; }

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

    static const std::vector<std::vector<point<int8_t>>> mates[12];

    static point_t getTileTypeOrigin(TileType t)
    {
        return origins[t];
    }

    static TileType getTileType( const point_t& p )
    {
        return (TileType) (std::find(
                origins, origins + numTileTypes() + 1,
                point<int8_t>{(int8_t) ((p.x_ % 7 + 7) % 7), (int8_t) ((p.y_ % 7 + 7) % 7)})
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

    static std::vector<int8_t> getBoundaryWordDirection( const point_t &dir ) {
        // A bit more complicated here, since drafters can have sides with the same orientation but different length
        auto it = std::find(boundaryWordDirections.begin(), boundaryWordDirections.end(), dir);
        if (it == boundaryWordDirections.end()) {
            // dir is a double-length side, so find the appropriate single-length direction and return it twice
            int8_t ans = std::find(boundaryWordDirections.begin(), boundaryWordDirections.end(), point_t{(coord) (dir.x_/2), (coord) (dir.y_/2)}) - boundaryWordDirections.begin();
            return {ans, ans};
        }
        return {(int8_t) (it - boundaryWordDirections.begin())};
    }

    static size_t numRotations() { return 6; }
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

    static point<double> vertexToGridCoords(point_t pt) {
        return {pt.x_ / 6.0, pt.y_ / 6.0};
    }

    static point<double> gridToPageCoords(point<double> pt) {
        const double sqrt3 = 1.73205080756887729353;
        xform<double> T{
            1.0, 1.0 / 2,0,
            0, sqrt3 / 2, 0};
        return T * pt;
    }

private:
    static std::vector<point_t> getTileVertices( const point_t& p )
    {
        const auto &vertexVecs = vertices[getTileType(p)];
        std::vector<point_t> ans(vertexVecs.size());
        point_t pTrans{(coord) (p.x_ * 6), (coord) (p.y_ * 6)};
        for (size_t i = 0; i < vertexVecs.size(); ++i)
            ans[i] = vertexVecs[i] + pTrans;
        return ans;
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
        {
                {2, 8}, {9, -6}, {-12, -6}
        },
        {
                {-6, 9}, {8, 2}, {-6, -12}
        },
        {
                {-8, 10}, {6, 3}, {6, -18}
        },
        {
                {-9, 3}, {-2, 10}, {12, -18}
        },
        {
                {-10, 2}, {-3, 9}, {18, -12}
        },
        {
                {-3, -6}, {-10, 8}, {18, -6}
        },
        {
                {-2, -8}, {-9, 6}, {12, 6}
        },
        {
                {6, -9}, {-8, -2}, {6, 12}
        },
        {
                {8, -10}, {-6, -3}, {-6, 18}
        },
        {
                {9, -3}, {2, -10}, {-12, 18}
        },
        {
                {10, -2}, {3, -9}, {-18, 12}
        },
        {
                {3, 6}, {10, -8}, {-18, 6}
        }
};

template<typename coord>
const std::vector<point<int8_t>> DrafterGrid<coord>::boundaryWordDirections = {
        {-7, 14}, {0, 21}, {7, 7}, {21, 0}, {14, -7}, {21, -21},
        {7, -14}, {0, -21}, {-7, -7}, {-21, 0}, {-14, 7}, {-21, 21},
};

template<typename coord>
const std::vector<std::vector<point<int8_t>>> DrafterGrid<coord>::mates[12] = {
        {
                {{2, 0}, {2, 1}},
                {{-1, 1}, {-1, 3}},
                {{-1, 1}, {2, 0}},
        },
        {
                {{0, 2}, {1, 2}},
                {{0, 2}, {1, -1}},
                {{1, -1}, {3, -1}}
        },
        {
                {{0, 2}, {-1, 3}},
                {{0, 2}, {-1, 0}},
                {{-1, 0}, {-3, 2}}
        },
        {
                {{1, 0}, {1, 2}},
                {{1, 0}, {-2, 2}},
                {{-2, 2}, {-2, 3}}
        },
        {
                {{-2, 2}, {-3, 2}},
                {{0, -1}, {-2, 2}},
                {{0, -1}, {-2, -1}}
        },
        {
                {{0, 1}, {-2, 3}},
                {{0, 1}, {-2, 0}},
                {{-2, 0}, {-3, 1}}
        },
        {
                {{-2, 0}, {-2, -1}},
                {{1, -1}, {1, -3}},
                {{1, -1}, {-2, 0}},
        },
        {
                {{0, -2}, {-1, -2}},
                {{0, -2}, {-1, 1}},
                {{-1, 1}, {-3, 1}}
        },
        {
                {{0, -2}, {1, -3}},
                {{0, -2}, {1, 0}},
                {{1, 0}, {3, -2}}
        },
        {
                {{-1, 0}, {-1, -2}},
                {{-1, 0}, {2, -2}},
                {{2, -2}, {2, -3}}
        },
        {
                {{2, -2}, {3, -2}},
                {{0, 1}, {2, -2}},
                {{0, 1}, {2, 1}}
        },
        {
                {{0, -1}, {2, -3}},
                {{0, -1}, {2, 0}},
                {{2, 0}, {3, -1}}
        }
};