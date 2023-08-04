#pragma once

#include <cstdint>
#include <iterator>

#include "geom.h"


template<typename coord>
class KiteGrid
{
public:
	using coord_t = coord;
	using point_t = point<coord>;
	using xform_t = xform<coord>;
    using edge_t = std::pair<point_t, point_t>;

public:
	static size_t numNeighbours( const point_t& p )
	{
		return 9;
	}

    static size_t getTileOrientation( const point_t& p )
    {
		const size_t idx = (((p.y_%6)+6)%6)*6 + (((p.x_%6)+6)%6);
		// std::cerr << "Tile orientation of " << p << " (" << idx << ") is " << tile_orientations[idx] << std::endl;
		return tile_orientations[idx];
    }

	static const point<int8_t> *getNeighbourVectors( const point_t& p )
	{
		return all_neighbours[getTileOrientation(p)];
	}

	static size_t numEdgeNeighbours( const point_t& p )
	{
		return 4;
	}

	static const point<int8_t> *getEdgeNeighbourVectors( const point_t& p )
	{
		return edge_neighbours[getTileOrientation(p)];
	}

	static bool translatable( const point_t& p, const point_t& q )
	{
		auto c = q.x_ - p.x_;
		auto d = q.y_ - p.y_;
		return ((d%2)==0) && (((c-d)%6)==0);
	}

	static std::vector<point_t> getCellVertices( const point_t& p )
    {
		size_t ori = getTileOrientation( p );
		point_t cen = p - orientation_offsets[ ori ];
		std::vector<point_t> ans( 4 );
		for( size_t idx = 0; idx < 4; ++idx ) {
			ans[idx] = cen + tile_vertices[ori][idx];
		}
			
        return ans;
    }

    static point<double> vertexToGrid( const point_t& pt )
	{
        return {pt.x_ / 2.0, pt.y_ / 2.0};
    }

    static point<double> gridToPage( const point<double>& pt )
	{
        const double sqrt3 = 1.73205080756887729353;
        xform<double> T{
                1.0, 1.0 / 2,0,
                0, sqrt3 / 2, 0};
        return T * pt;
    }

	static const size_t num_orientations;
	static const xform<int8_t> orientations[12];
	
	static const point<int8_t> edge_neighbours[6][4];
	static const point<int8_t> all_neighbours[6][9];

	static const size_t tile_orientations[36];
	static const point<int8_t> orientation_offsets[6];
	static const point<int8_t> tile_vertices[6][4];
};

template<typename coord>
const size_t KiteGrid<coord>::num_orientations = 12;

template<typename coord>
const xform<int8_t> KiteGrid<coord>::orientations[12] = {
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

// A magic lookup table that tells you the orientation of each
// kite in a 6x6 paralellogram at the origin. 7s correspond to 
// hex cells that aren't kites.  Orientations start at east=0
// and continue CCW from there.
template<typename coord>
const size_t KiteGrid<coord>::tile_orientations[36] = {
	7, 0, 7, 7, 7, 3,
	1, 7, 4, 5, 7, 2,
	7, 3, 7, 0, 7, 7, 
	7, 2, 1, 7, 4, 5,
	7, 7, 7, 3, 7, 0,
	4, 5, 7, 2, 1, 7
};

// For each orientation (CCW starting from East), give an offset
// vector from the center of the cluster to the tile with that orientation.
template<typename coord>
const point<int8_t> KiteGrid<coord>::orientation_offsets[6] = {
	{1,0}, {0,1}, {-1,1}, {-1,0}, {0,-1}, {1,-1}
};

template<typename coord>
const point<int8_t> KiteGrid<coord>::tile_vertices[6][4] = {
	{ // east
		{ 0, 0 },
		{ 2, -1 },
		{ 2, 0 },
		{ 1, 1 },
	},
	{ // northeast
		{ 0, 0 },
		{ 1, 1 },
		{ 0, 2 },
		{ -1, 2 }
	},
	{ // northwest
		{ 0, 0 },
		{ -1, 2 },
		{ -2, 2 },
		{ -2, 1 }
	},
	{ // west
		{ 0, 0 },
		{ -2, 1 },
		{ -2, 0 },
		{ -1, -1 }
	},
	{ // southwest
		{ 0, 0 },
		{ -1, -1 },
		{ 0, -2 },
		{ 1, -2 }
	},
	{ // southeast
		{ 0, 0 },
		{ 1, -2 },
		{ 2, -2 },
		{ 2, -1 }
	}
};

template<typename coord>
const point<int8_t> KiteGrid<coord>::edge_neighbours[6][4] = {
    { // east
        { 1, 1 },
        { 2, -1 },
        { -1, 1 },
        { 0, -1 }
    },
    { // northeast
        { -1, 2 },
        { 1, 1 },
        { -1, 0 },
        { 1, -1 }
    },
    { // northwest
        { -2, 1 },
        { -1, 2 },
        { 0, -1 },
        { 1, 0 }
    },
    { // west
        { -1, -1 },
        { -2, 1 },
        { 1, -1 },
        { 0, 1 }
    },
    { // southwest
        { 1, -2 },
        { -1, -1 },
        { 1, 0 },
        { -1, 1 }
    },
    { // southeast
        { 2, -1 },
        { 1, -2 },
        { 0, 1 },
        { -1, 0 }
    }
};

template<typename coord>
const point<int8_t> KiteGrid<coord>::all_neighbours[6][9] = {
    { // east
        { 1, 1 },
        { 2, -1 },
        { -1, 1 },
        { 0, -1 },
        { 0, 2 },
        { 2, -2 },
        { -2, 0 },
        { -2, 1 },
        { -1, -1 }
    },
    { // northeast
        { -1, 2 },
        { 1, 1 },
        { -1, 0 },
        { 1, -1 },
        { -2, 2 },
        { 2, 0 },
        { 0, -2 },
        { -1, -1 },
        { 1, -2 }
    },
    { // northwest
        { -2, 1 },
        { -1, 2 },
        { 0, -1 },
        { 1, 0 },
        { -2, 0 },
        { 0, 2 },
        { 2, -2 },
        { 1, -2 },
        { 2, -1 }
    },
    { // west
        { -1, -1 },
        { -2, 1 },
        { 1, -1 },
        { 0, 1 },
        { 0, -2 },
        { -2, 2 },
        { 2, 0 },
        { 2, -1 },
        { 1, 1 }
    },
    { // southwest
        { 1, -2 },
        { -1, -1 },
        { 1, 0 },
        { -1, 1 },
        { 2, -2 },
        { -2, 0 },
        { 0, 2 },
        { 1, 1 },
        { -1, 2 }
    },
    { // southeast
        { 2, -1 },
        { 1, -2 },
        { 0, 1 },
        { -1, 0 },
        { 2, 0 },
        { 0, -2 },
        { -2, 2 },
        { -1, 2 },
        { -2, 1 }
    }
};
