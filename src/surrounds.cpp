#include <iostream>
#include <cstdint>
#include <sstream>
#include <map>

#include "heesch.h"
#include "grid.h"
#include "tileio.h"
#include "cloud.h"

// Enumerate all surrounds of a given polyform.

using namespace std;

static bool no_reflections = false;
static bool extremes = false;
static size_t heesch_level = 1;

template<typename grid>
static size_t countEquivalentOrientations( const TileInfo<grid>& tile )
{
	Shape<grid> shape { tile.getShape() };
	shape.untranslate();

	Shape<grid> ts;

	size_t equiv = 0;

	for( size_t idx = 0; idx < grid::num_orientations; ++idx ) {
		auto T { grid::orientations[idx] };
		ts.reset( shape, T );
		ts.untranslate();

		if( ts == shape ) {
			++equiv;
		}
	}

	return equiv;
}

template<typename grid>
static bool describeNeighbours( const TileInfo<grid>& tile )
{
	size_t eq = countEquivalentOrientations( tile );

	Cloud<grid> cloud { tile.getShape() };
	size_t sz = cloud.adjacent_.size();

	// FIXME: This should probably be (sz/(eq*eq)), not (sz/eq). 
	// As written, I think this factors out symmetries of the 
	// neighbour, but not symmetries of the original tile.
	cout << (sz/eq) << " adjacents, " << sz << " ignoring symmetries ";
	tile.write( cout );
//	cout << endl;
	return true;
}
GRID_WRAP( describeNeighbours );

template<typename grid>
static bool countSurrounds( const TileInfo<grid>& tile )
{
	using coord_t = typename grid::coord_t;

	TileInfo<grid> info { tile };

	map<size_t,size_t> counts;
	size_t num = 0;

	HeeschSolver<grid> solver { 
		info.getShape(), no_reflections ? TRANSLATIONS_ROTATIONS : ALL };
		
	for( size_t idx = 0; idx < heesch_level; ++idx ) {
		solver.increaseLevel();
	}

	solver.allCoronas( [&counts, &num]( const Solution<coord_t>& soln ) {
		counts[soln.size()-1]++; 
		++num; 
		/*
		if( soln.size() == 15 ) {
			info.setNonTiler( 1, &soln, 1, nullptr );
			info.write( cout );
		}
		*/
		if( num % 100000 == 0 ) {
			cerr << ".";
		}
		return true; } );

	cerr << endl;

	for( const auto& p : counts ) {
		cout << p.second << " surrounds of size " << p.first << endl;
	}

	return true;
}
GRID_WRAP( countSurrounds );

template<typename grid>
static bool computeSurrounds( const TileInfo<grid>& tile )
{
	using coord_t = typename grid::coord_t;

	tile.getShape().debug();

	TileInfo<grid> info { tile };

	HeeschSolver<grid> solver { 
		info.getShape(), no_reflections ? TRANSLATIONS_ROTATIONS : ALL };
		
	for( size_t idx = 0; idx < heesch_level; ++idx ) {
		solver.increaseLevel();
	}

	std::vector<Solution<coord_t>> cur;
	solver.allCoronas( cur );

	if( extremes ) {
		Solution<coord_t> smallest = cur[0];
		Solution<coord_t> largest = cur[0];

		for( const auto& soln : cur ) {
			if( soln.size() < smallest.size() ) {
				smallest = soln;
			} else if( soln.size() > largest.size() ) {
				largest = soln;
			}
		}

		info.setNonTiler( 1, &smallest, 1, nullptr );
		info.write( cout );
		info.setNonTiler( 1, &largest, 1, nullptr );
		info.write( cout );
	} else {
		for( const auto& soln : cur ) {
			info.setNonTiler( 1, &soln, 1, nullptr );
			info.write( cout );
		}
	}
	return true;
}
GRID_WRAP( computeSurrounds );

int main( int argc, char **argv )
{
	bool count = false;
	bool neighs = false;

	for( int idx = 1; idx < argc; ++idx ) {
		if( !strcmp( argv[idx], "-level" ) ) {
		    heesch_level = atoi(argv[idx+1]);
		    ++idx;
		} else if( !strcmp( argv[idx], "-noreflections" ) ) {
		    no_reflections = true;
		} else if( !strcmp( argv[idx], "-extremes" ) ) {
		    extremes = true;
		} else if( !strcmp( argv[idx], "-count" ) ) {
		    count = true;
		} else if( !strcmp( argv[idx], "-neighbours" ) ) {
		    neighs = true;
		} else {
			cerr << "Unrecognized parameter \"" << argv[idx] << "\""
				<< endl;
			exit( 0 );
		}
	}

	if( neighs ) {
		FOR_EACH_IN_STREAM( cin, describeNeighbours );
	} else if( count ) {
		FOR_EACH_IN_STREAM( cin, countSurrounds );
	} else {
		FOR_EACH_IN_STREAM( cin, computeSurrounds );
	}
	return 0;
}
