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

    enum TileType {
		INVALID = -1,
		HEXAGON = 0
    };

    enum TileShape {
		HEXAGON_SHAPE = 0
    };

public:
	inline static GridType grid_type = HEX;

    inline static size_t num_tile_types = 1; 
    inline static size_t num_tile_shapes = 1;

	inline static TileType getTileType( const point_t& p )
	{
		return HEXAGON;
	}
	inline static TileShape getTileShape( const point_t& p )
	{
		return HEXAGON_SHAPE;
	}

	inline static point_t getOrigin( const point_t& p )
	{
		return { 0, 0 };
	}

	inline static size_t numNeighbours( const point_t& p )
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

	static size_t numVertices(const point_t& p)
	{
		return 6;
	}

	static point_t getVertexCentre(const point_t& p)
	{
		return point_t {
			static_cast<coord_t>(3 * p.getX()), 
			static_cast<coord_t>(3 * p.getY())};
	}

	static const point<int8_t> *getVertexVectors(const point_t& p)
	{
		return vertex_neighbours;
	}

	static void getCellAroundVertexInfo(
		const point_t& p, point_t& cen, const point<int8_t> *& pts, size_t& num)
	{
		num = 3;
		coord_t c = (((p.getX() + p.getY()) % 3) + 3) % 3;
		if (c == 2) {
			cen = point_t {(p.getX() - 1) / 3, (p.getY() - 1) / 3};
			pts = vertex_cell_neighbours[0];
		} else {
			cen = point_t {(p.getX() - 2) / 3, (p.getY() + 1) / 3};
			pts = vertex_cell_neighbours[1];
		}
	}

	static point<double> vertexToGrid( const point_t& pt ) 
	{
		return point<double>( 
			static_cast<double>(pt.getX()) / 3.0,
			static_cast<double>(pt.getY()) / 3.0 );
	}

	static point<double> gridToPage( const point<double>& pt )
	{
		const double sqrt3 = 1.73205080756887729353;
		return { pt.getX() + 0.5 * pt.getY(), sqrt3 * pt.getY() / 2.0 };
	}

	static const point_t origins[1];

	static const size_t num_orientations = 12;
	static const xform<int8_t> orientations[12];
	
	static const point<int8_t> all_neighbours[6];
	static const point<int8_t> vertex_neighbours[6];
	static const point<int8_t> vertex_cell_neighbours[2][3];

	static const point_t translationV1;
	static const point_t translationV2;
};

template<typename coord>
const point<coord> HexGrid<coord>::origins[1] = {
	{ 0, 0 }
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
const point<int8_t> HexGrid<coord>::vertex_neighbours[6] = {
		{1, 1},
		{-1, 2},
		{-2, 1},
		{-1, -1},
		{1, -2},
		{2, -1}};

template<typename coord>
const point<int8_t> HexGrid<coord>::vertex_cell_neighbours[2][3] = {
	{{0, 0}, {1, 0}, {0, 1}},
	{{0, 0}, {1, -1}, {1, 0}}};

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

template<typename coord>
const point<coord> HexGrid<coord>::translationV1 {1, 0};

template<typename coord>
const point<coord> HexGrid<coord>::translationV2 {0, 1};
