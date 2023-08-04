#pragma once

#include <cstdint>
#include <iterator>

#include "geom.h"

template<typename coord>
class OminoGrid
{
public:
	using coord_t = coord;
	using point_t = point<coord>;
	using xform_t = xform<coord>;
	using edge_t = std::pair<point_t, point_t>;

public:
	static size_t numNeighbours( const point_t& p )
	{
		return 8;
	}

	static const point<int8_t> *getNeighbourVectors( const point_t& p )
	{
		return all_neighbours;
	}

	static size_t numEdgeNeighbours( const point_t& p )
	{
		return 4;
	}

	static const point<int8_t> *getEdgeNeighbourVectors( const point_t& p )
	{
		return edge_neighbours;
	}

	static bool translatable( const point_t& p, const point_t& q )
	{
		return true;
	}

	static std::vector<edge_t> getCellEdges( const point_t& p )
	{
		coord_t x = p.getX();
		coord_t xp1 = static_cast<coord_t>( x + 1 );
		coord_t y = p.getY();
		coord_t yp1 = static_cast<coord_t>( y + 1 );

		return std::vector<edge_t> {
			{ { x, y }, { xp1, y } },
			{ { xp1, y }, { xp1, yp1 } },
			{ { xp1, yp1 }, { x, yp1 } },
			{ { x, yp1 }, { x, y } } };
	}

	static point<double> vertexToGrid( const point_t& pt ) 
	{
		return point<double>( 
			static_cast<double>(pt.getX()) - 0.5,
			static_cast<double>(pt.getY()) - 0.5 );
	}

	static point<double> gridToPage( const point<double>& pt )
	{
		return pt;
	}

	static const size_t num_orientations;
	static const xform<int8_t> orientations[8];
	
	static const point<int8_t> all_neighbours[8];
	static const point<int8_t> edge_neighbours[4];
};

template<typename coord>
const point<int8_t> OminoGrid<coord>::all_neighbours[8] = {
		{ -1, -1 },
		{ 0, -1 },
		{ 1, -1 },
		{ -1, 0 },
		{ 1, 0 },
		{ -1, 1 },
		{ 0, 1 },
		{ 1, 1 } };

template<typename coord>
const point<int8_t> OminoGrid<coord>::edge_neighbours[4] = {
		{ 0, -1 },
		{ -1, 0 },
		{ 1, 0 },
		{ 0, 1 } };

template<typename coord>
const size_t OminoGrid<coord>::num_orientations = 8;

template<typename coord>
const xform<int8_t> OminoGrid<coord>::orientations[8] = {
	{ 1, 0, 0, 0, 1, 0 }, { 0, -1, 0, 1, 0, 0 }, 
	{ -1, 0, 0, 0, -1, 0 }, { 0, 1, 0, -1, 0, 0 },
	{ -1, 0, 0, 0, 1, 0 }, { 0, -1, 0, -1, 0, 0 },
	{ 1, 0, 0, 0, -1, 0 }, { 0, 1, 0, 1, 0, 0 } };
