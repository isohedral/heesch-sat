#pragma once

#include <vector>

#include "shape.h"

// Function to extract the boundary of a polyform.  Nicely linear time, but it would be nice
// to find a way to do this without quite so much dynamic memory.  Could be worse, given that
// you're likely to use this just once per shape.

template<typename coord_t>
using boundary_edge_map = 
	point_map<coord_t, std::pair<point<coord_t>, point<coord_t>>>;

template<typename grid>
boundary_edge_map<typename grid::coord_t> getTileEdgeMap(
	const Shape<grid>& shape)
{
	using point_t = typename grid::point_t;
	
	boundary_edge_map<typename grid::coord_t> edges;

	point_t vs[MAX_CELL_SIZE];

	for (const auto& p: shape) {
		vertices<grid> v {p};
		size_t num = 0;
		for (const auto& v: vertices<grid> {p}) {
			vs[num++] = v;
		}

		point_t prev = vs[num - 1];
		for (size_t idx = 0; idx < num; ++idx) {
			point_t cur = vs[idx];
			point_t mid = prev + cur;

			if (edges.find(mid) == edges.end()) {
				edges[mid] = std::make_pair(prev, cur);
			} else {
				edges.erase(mid);
			}

			prev = cur;
		}
	}

	return edges;
}

// Extract the pseudo-vertices making up the boundary of the tile.
// The behaviour of this function is undefined if the tile isn't
// simply connected.
template<typename grid>
std::vector<typename grid::point_t> getTileBoundary(const Shape<grid>& shape)
{
	// For now, use a silly algorithm based on auxiliary set and map
	// data structures.  A little slower than necessary, and definitely
	// wasteful in terms of dynamic memory, but simple to implement.
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;
	
	// Step 1: Get a set of unpaired edges by scanning every edge of every 
	// cell in the shape
	boundary_edge_map<typename grid::coord_t> edges = getTileEdgeMap(shape);

	// Step 2: Turn this into a map from start vertices to end vertices
	point_map<coord_t, point_t> next;
	for (const auto& i: edges) {
		next[i.second.first] = i.second.second;
	}

	// Step 3: Use the map to reconstruct the boundary
	std::vector<point_t> ret;

	point_t start = next.begin()->first;
	point_t v = start;

	do {
		ret.push_back(v);
		v = next[v];
	} while (v != start);

	return ret;
}
