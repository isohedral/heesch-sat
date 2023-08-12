#pragma once

#include <cstdint>
#include <iterator>

#include "geom.h"

template<typename coord>
class OctaSquareGrid
{
public:
	using coord_t = coord;
	using point_t = point<coord>;
	using xform_t = xform<coord>;
	using edge_t = std::pair<point_t, point_t>;

    enum TileType {
        SQUARE, OCTAGON
    };

    enum TileShape {
        SQUARE_SHAPE, OCTAGON_SHAPE
    };

public:
    static size_t numTileTypes() { return 2; }
    static size_t numTileShapes() { return 2; }

	static size_t numNeighbours( const point_t& p )
	{
		return getTileType(p) == SQUARE ? 4 : 8;
	}

	static const point<int8_t> *getNeighbourVectors( const point_t& p )
	{
        return getTileType(p) == SQUARE ? edge_neighbours : all_neighbours;
	}

	static size_t numEdgeNeighbours( const point_t& p )
	{
        return getTileType(p) == SQUARE ? 4 : 8;
	}

	static const point<int8_t> *getEdgeNeighbourVectors( const point_t& p )
	{
        return getTileType(p) == SQUARE ? edge_neighbours : all_neighbours;
	}

	static bool translatable( const point_t& p, const point_t& q )
	{
		return getTileType(p) == getTileType(q);
	}

	static const point_t origins[2];

	static const size_t num_orientations;
	static const xform<int8_t> orientations[8];
	
	static const point<int8_t> all_neighbours[8];
	static const point<int8_t> edge_neighbours[4];

	static const std::vector<point<int8_t>> squareVertices;
    static const std::vector<point<int8_t>> octagonVertices;

    static const std::vector<point<int8_t>> boundaryWordDirections;

    static point_t getTileTypeOrigin(TileType t)
    {
        return t == SQUARE ? point_t{0, 0} : point_t{1, 0};
    }

    static TileType getTileType( const point_t& p )
    {
        return (p.x_ + p.y_) % 2 == 0 ? SQUARE : OCTAGON;
    }

    static TileShape getTileShape( const TileType t )
    {
        return (TileShape) t;
    }

    static TileShape getTileShape( const point_t& p )
    {
        return getTileShape(getTileType(p));
    }

    static std::vector<point_t> getCellVertices( const point_t& p )
    {
        const auto &vertexVecs = (getTileType(p) == SQUARE ? squareVertices : octagonVertices);

        std::vector<point_t> ans { vertexVecs.size() };
        for (size_t i = 0; i < vertexVecs.size(); ++i) {
            ans[i] = p + p + vertexVecs[i];
		}
        return ans;
    }

    // assumes dir is a valid direction
    static int8_t getBoundaryWordDirection( const point_t dir ) {
        return std::find(boundaryWordDirections.begin(), boundaryWordDirections.end(), dir) - boundaryWordDirections.begin();
    }

    static size_t numRotations() { return 4; }
    static int8_t rotateDirection( int8_t dir ) { return (dir + 2) % boundaryWordDirections.size(); }
    static int8_t reflectDirection( int8_t dir ) { return (boundaryWordDirections.size() - dir) % boundaryWordDirections.size(); }

    static point<double> vertexToGrid( const point_t& pt ) {
		// (sqrt(2) − 1) / (2 + 2*sqrt(2))
        const double shift = 0.0857864376269049512; 

        // Shift vertices to make the octagons regular, instead of 
		// having edges of length 1 and sqrt(2).
        double x = pt.x_ % 2 == 0 ? pt.x_ - shift : pt.x_ + shift;
        double y = pt.y_ % 2 == 0 ? pt.y_ - shift : pt.y_ + shift;
        return {x / 2.0 - 0.25, y / 2.0 - 0.25};
    }

    static point<double> gridToPage( const point<double>& pt) {
        return pt;
    }

    static bool hasMates() { return false; }

    static std::vector<std::vector<point_t>> getMatesList(const point_t &p) {
        return {{}};
    }

private:
    static std::vector<point_t> getTileVertices( const point_t& p )
    {
        const auto &vertexVecs = (getTileType(p) == SQUARE ? squareVertices : octagonVertices);
        std::vector<point_t> ans(vertexVecs.size());
        for (size_t i = 0; i < vertexVecs.size(); ++i)
            ans[i] = p + p + vertexVecs[i];
        return ans;
    }
};

template<typename coord>
const point<coord> OctaSquareGrid<coord>::origins[2] = {
	{ 0, 0 }, 
	{ 1, 0 }, 
};

template<typename coord>
const point<int8_t> OctaSquareGrid<coord>::all_neighbours[8] = {
		{ -1, -1 },
		{ 0, -1 },
		{ 1, -1 },
		{ -1, 0 },
		{ 1, 0 },
		{ -1, 1 },
		{ 0, 1 },
		{ 1, 1 } };

template<typename coord>
const point<int8_t> OctaSquareGrid<coord>::edge_neighbours[4] = {
		{ 0, -1 },
		{ -1, 0 },
		{ 1, 0 },
		{ 0, 1 } };

template<typename coord>
const size_t OctaSquareGrid<coord>::num_orientations = 8;

template<typename coord>
const xform<int8_t> OctaSquareGrid<coord>::orientations[8] = {
	{ 1, 0, 0, 0, 1, 0 }, { 0, -1, 0, 1, 0, 0 }, 
	{ -1, 0, 0, 0, -1, 0 }, { 0, 1, 0, -1, 0, 0 },
	{ -1, 0, 0, 0, 1, 0 }, { 0, -1, 0, -1, 0, 0 },
	{ 1, 0, 0, 0, -1, 0 }, { 0, 1, 0, 1, 0, 0 } };

template<typename coord>
const std::vector<point<int8_t>> OctaSquareGrid<coord>::squareVertices = {
        {0, 0}, {0, 1}, {1, 1}, {1, 0}};

template<typename coord>
const std::vector<point<int8_t>> OctaSquareGrid<coord>::octagonVertices = {
        {0, -1}, {-1, 0}, {-1, 1}, {0, 2}, {1, 2}, {2, 1}, {2, 0}, {1, -1}};

template<typename coord>
const std::vector<point<int8_t>> OctaSquareGrid<coord>::boundaryWordDirections = {
        {1, 0}, {1, -1}, {0, -1}, {-1, -1}, {-1, 0}, {-1, 1}, {0, 1}, {1, 1}};
