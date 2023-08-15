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

public:
	// Number of transivity classes of tiles under translation
    inline static size_t num_tile_types = 1; 
	// Number of distinct shapes 
    inline static size_t num_tile_shapes = 1;
	// What tile type is the tile indexed by p?
	inline static TileType getTileType( const point_t& p )
	{
		return SQUARE;
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

	// Functions to assist with rendering

	// Get points with integer coordinates corresponding to the *vertices*
	// of the cell indexed by p.  These can really be in any coordinate
	// system whatsoever -- they just need to be in one-to-one correspondence
	// with the actual vertices, so that they can be compared exactly.
	static std::vector<point_t> getCellVertices( const point_t& p )
	{
		return {
			p, 
			p + point_t { 1, 0 },
			p + point_t { 1, 1 },
			p + point_t { 0, 1 } };
	}

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

	static const size_t num_orientations;
	static const xform<int8_t> orientations[8];
	
	static const point<int8_t> all_neighbours[8];
	static const point<int8_t> edge_neighbours[4];
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
const size_t OminoGrid<coord>::num_orientations = 8;

template<typename coord>
const xform<int8_t> OminoGrid<coord>::orientations[8] = {
	{ 1, 0, 0, 0, 1, 0 }, { 0, -1, 0, 1, 0, 0 }, 
	{ -1, 0, 0, 0, -1, 0 }, { 0, 1, 0, -1, 0, 0 },
	{ -1, 0, 0, 0, 1, 0 }, { 0, -1, 0, -1, 0, 0 },
	{ 1, 0, 0, 0, -1, 0 }, { 0, 1, 0, 1, 0, 0 } };
