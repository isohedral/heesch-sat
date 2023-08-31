#pragma once

#include <cstdint>
#include <iterator>

#include "geom.h"

template<typename coord>
class IamondGrid
{
public:
	using coord_t = coord;
	using point_t = point<coord>;
	using xform_t = xform<coord>;

    enum TileType {
		INVALID = -1,
		TRIANGLE_UP = 0,
		TRIANGLE_DOWN = 1,
    };

    enum TileShape {
		TRIANGLE_SHAPE = 0,
    };

public:
	inline static GridType grid_type = IAMOND;

    inline static size_t num_tile_types = 2; 
    inline static size_t num_tile_shapes = 1;

	inline static TileType getTileType( const point_t& p )
	{
		return ((p.x_ % 3) == 0) ? TRIANGLE_UP : TRIANGLE_DOWN;
	}
	inline static TileShape getTileShape( const point_t& p )
	{
		return TRIANGLE_SHAPE;
	}

	inline static point_t getOrigin( const point_t& p )
	{
		return origins[ (size_t)getTileType( p ) ];
	}

	inline static size_t numNeighbours( const point_t& p )
	{
		return 12;
	}

	static bool isBlack( const point_t& p ) 
	{
		return (p.x_ % 3) == 0;
	}

	static const point<int8_t> *getNeighbourVectors( const point_t& p )
	{
		if( isBlack( p ) ) {
			return all_neighbours_black;
		} else {
			return all_neighbours_grey;
		}
	}

	static size_t numEdgeNeighbours( const point_t& p )
	{
		return 3;
	}

	static const point<int8_t> *getEdgeNeighbourVectors( const point_t& p )
	{
		if( isBlack( p ) ) {
			return edge_neighbours_black;
		} else {
			return edge_neighbours_grey;
		}
	}

	static bool translatable( const point_t& p, const point_t& q )
	{
		return ((p.x_-q.x_) % 3) == 0;
	}

	static std::vector<point_t> getCellVertices( const point_t& p )
	{
		if( isBlack( p ) ) {
			return { 
				p + point_t { -1, 2 },
				p + point_t { -1, -1 },
				p + point_t { 2, -1 } };
		} else {
			return { 
				p + point_t { 1, 1 },
				p + point_t { -2, 1 },
				p + point_t { 1, -2 } };
		}
	}

	static point<double> vertexToGrid( const point_t& pt ) 
	{
		return point<double>( 
			static_cast<double>(pt.getX()),
			static_cast<double>(pt.getY()) );
	}

	static point<double> gridToPage( const point<double>& pt )
	{
		const double sqrt3 = 1.73205080756887729353;
		return { pt.getX() + 0.5 * pt.getY(), sqrt3 * pt.getY() / 2.0 };
	}

	static const point_t origins[2];

	static const size_t num_orientations;
	static const xform<int8_t> orientations[12];
	
	static const point<int8_t> all_neighbours_black[12];
	static const point<int8_t> all_neighbours_grey[12];
	static const point<int8_t> edge_neighbours_black[3];
	static const point<int8_t> edge_neighbours_grey[3];
};

template<typename coord>
const point<coord> IamondGrid<coord>::origins[2] = {
	{ 0, 0 }, 
	{ 1, -2 }
};

template<typename coord>
const point<int8_t> IamondGrid<coord>::all_neighbours_black[12] =
    { { 3, 0 }, { 0, 3 }, { -3, 3 }, { -3, 0 }, { 0, -3 }, { 3, -3 },
      { 1, 1 }, { -2, 4 }, { -2, 1 }, { -2, -2 }, { 1, -2 }, { 4, -2 } };
template<typename coord>
const point<int8_t> IamondGrid<coord>::all_neighbours_grey[12] =
    { { 3, 0 }, { 0, 3 }, { -3, 3 }, { -3, 0 }, { 0, -3 }, { 3, -3 },
      { 2, 2 }, { 2, -1 }, { 2, -4 }, { -1, -1 }, { -4, 2 }, { -1, 2 } };

template<typename coord>
const point<int8_t> IamondGrid<coord>::edge_neighbours_black[3] =
    { { 1, 1 }, { -2, 1 }, { 1, -2 } };
template<typename coord>
const point<int8_t> IamondGrid<coord>::edge_neighbours_grey[3] =
    { { -1, -1 }, { 2, -1 }, { -1, 2 } };

template<typename coord>
const size_t IamondGrid<coord>::num_orientations = 12;

template<typename coord>
const xform<int8_t> IamondGrid<coord>::orientations[12] = {
      { 1, 0, 0,    0, 1, 0 },
      { -1, -1, 0,  1, 0, 0 },
      { 0, 1, 0,    -1, -1, 0 },
      { 1, 0, 0,    -1, -1, 0 },
      { 0, 1, 0,    1, 0, 0 },
      { -1, -1, 0,  0, 1, 0 },
      { 0, -1, 1,   -1, 0, 1 },
      { -1, 0, 1,   1, 1, 1 },
      { 1, 1, 1,    0, -1, 1 },
      { 1, 1, 1,    -1, 0, 1 },
      { -1, 0, 1,   0, -1, 1 },
      { 0, -1, 1,   1, 1, 1 } };
