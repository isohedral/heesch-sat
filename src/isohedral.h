#pragma once 

#include <vector>

#include "geom.h"
#include "words.h"

// This is currently somewhat pointless as a class, but keep it that way
// for now.  Later it may be useful to include some persistent memory 
// resources in instances of this class to avoid too much dynamic memory
// when checking many shapes.
class IsohedralChecker {
public:
	IsohedralChecker()
	{}

	// Order is largest rotational order supported by the grid used to
	// describe this shape.  Should be 2, 4, or 6.  Note that the boundary word
	// is assumed to describe the outline of the shape in *counterclockwise*
	// order. Passing in a clockwise boundary is not guaranteed to detect
	// all isohedral types correctly.

	// This function will sometimes produce false positives relative to a
	// the area-based (i.e., SAT-based) isohedral checker.  It can find 
	// tilings by a polyform that aren't grid-aligned for that polyform's
	// grid.  Consider, e.g., a single hexagon in the bevelhex grid.
	// (That won't turn up when running sat naively, because it will be
	// ruled out before the isohedral check by virtue of being 
	// unsurroundable.  But the point still holds, and manifests itself
	// quickly for some polydrafters and poly-[3.6.3.6], for instance.)
	bool tilesIsohedrally(
		const std::vector<point<int8_t>>& w, size_t order = 2);
};
