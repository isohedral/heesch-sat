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

public:
	static size_t numNeighbours( const point_t& p )
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

	static const size_t num_orientations;
	static const xform<int8_t> orientations[12];
	
	static const point<int8_t> all_neighbours_black[12];
	static const point<int8_t> all_neighbours_grey[12];
	static const point<int8_t> edge_neighbours_black[3];
	static const point<int8_t> edge_neighbours_grey[3];
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
