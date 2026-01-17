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

    enum TileType {
		INVALID = -1,
		SQUARE = 0
    };

    enum TileShape {
		SQUARE_SHAPE = 0
	};

public:
	inline static GridType grid_type = OMINO;

	// Number of transivity classes of tiles under translation
    inline static size_t num_tile_types = 1; 
	// Number of distinct shapes 
    inline static size_t num_tile_shapes = 1;
	// What tile type is the tile indexed by p?
	inline static TileType getTileType( const point_t& p )
	{
		return SQUARE;
	}
	// What shape tile is at this position?
	inline static TileShape getTileShape( const point_t& p )
	{
		return SQUARE_SHAPE;
	}
	// Get the origin point 
	inline static point_t getOrigin( const point_t& p )
	{
		return { 0, 0 };
	}

	inline static size_t numNeighbours( const point_t& p )
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
	
	// Functions that give useful (but geometrically inaccurate) coordinates
	// for the vertices of cells.  These "points" need only be in one-to-one
	// correspondence to the actual vertices, and are always represented in
	// integer coordinates.  These are used with the "vertices" view defined
	// in grid.h.

	// How many vertices does the cell at position p have?
	static size_t numVertices(const point_t& p)
	{
		return 4;
	}

	// In some grids, you'll want to compute the coordinates for vertices
	// relative to some transformed version of p.  This function can provide
	// that if necessary.  (It's not necessary for ominos.)
	static point_t getVertexCentre(const point_t& p)
	{
		return p;
	}

	// For the same point p as above, retrieve a const array of vectors
	// that take you from getVertexCentre(p) to each of the vertices associated
	// with this cell.
	static const point<int8_t> *getVertexVectors(const point_t& p)
	{
		return vertex_neighbours;
	}

	// Need to be able to go backwards too: given a vertex location as defined
	// above, get a list of grid cells around it, in CCW order.
	static void getCellAroundVertexInfo(
		const point_t& p, point_t& cen, const point<int8_t> *& pts, size_t& num)
	{
		num = 4;
		cen = p;
		pts = vertex_cell_neighbours;
	}

	// Functions to assist with rendering

	// Convert a vertex as given by the previous function into a 2D
	// point that's compatible with the matrices giving the symmetries
	// of the grid.
	static point<double> vertexToGrid( const point_t& pt ) 
	{
		return point<double>( 
			static_cast<double>(pt.getX()) - 0.5,
			static_cast<double>(pt.getY()) - 0.5 );
	}

	// Any final cleanup of a point given by the previous function.
	static point<double> gridToPage( const point<double>& pt )
	{
		return pt;
	}

	static const point_t origins[1];

	static const size_t num_orientations = 8;
	static const xform<int8_t> orientations[8];
	
	static const point<int8_t> all_neighbours[8];
	static const point<int8_t> edge_neighbours[4];
	static const point<int8_t> vertex_neighbours[4];
	static const point<int8_t> vertex_cell_neighbours[4];

	static const point_t translationV1;
	static const point_t translationV2;
};

template<typename coord>
const point<coord> OminoGrid<coord>::origins[1] = {
	{ 0, 0 }
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
const point<int8_t> OminoGrid<coord>::vertex_neighbours[4] = {
		{0, 0},
		{1, 0},
		{1, 1},
		{0, 1}};

template<typename coord>
const point<int8_t> OminoGrid<coord>::vertex_cell_neighbours[4] = {
		{0, 0},
		{-1, 0},
		{-1, -1},
		{0, -1}};

template<typename coord>
const xform<int8_t> OminoGrid<coord>::orientations[8] = {
	{ 1, 0, 0, 0, 1, 0 }, { 0, -1, 0, 1, 0, 0 }, 
	{ -1, 0, 0, 0, -1, 0 }, { 0, 1, 0, -1, 0, 0 },
	{ -1, 0, 0, 0, 1, 0 }, { 0, -1, 0, -1, 0, 0 },
	{ 1, 0, 0, 0, -1, 0 }, { 0, 1, 0, 1, 0, 0 } };

template<typename coord>
const point<coord> OminoGrid<coord>::translationV1 {1, 0};

template<typename coord>
const point<coord> OminoGrid<coord>::translationV2 {0, 1};
