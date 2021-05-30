#pragma once

#include <cstdint>
#include <iterator>

#include "geom.h"

template<typename coord>
class HexGrid
{
public:
	using coord_t = coord;
	using point_t = point<coord>;
	using xform_t = xform<coord>;

public:
	static size_t numNeighbours( const point_t& p )
	{
		return 6;
	}

	static const point<int8_t> *getNeighbourVectors( const point_t& p )
	{
		return all_neighbours;
	}

	static size_t numEdgeNeighbours( const point_t& p )
	{
		return 6;
	}

	static const point<int8_t> *getEdgeNeighbourVectors( const point_t& p )
	{
		return all_neighbours;
	}

	static bool translatable( const point_t& p, const point_t& q )
	{
		return true;
	}

	static const size_t num_orientations;
	static const xform<int8_t> orientations[12];
	
	static const point<int8_t> all_neighbours[6];
};

template<typename coord>
const point<int8_t> HexGrid<coord>::all_neighbours[6] = {
		{ 0, -1 },
		{ 0, 1 },
		{ 1, 0 },
		{ -1, 0 },
		{ 1, -1 },
		{ -1, 1 } };

template<typename coord>
const size_t HexGrid<coord>::num_orientations = 12;

template<typename coord>
const xform<int8_t> HexGrid<coord>::orientations[12] = {
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

