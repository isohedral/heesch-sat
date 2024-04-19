#include <iostream>
#include <cstdint>
#include <sstream>

#include "heesch.h"
#include "grid.h"
#include "tileio.h"

// Enumerate all surrounds of a given polyform.

using namespace std;

static bool no_reflections = false;
static bool extremes = false;
static size_t heesch_level = 1;

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
	for( size_t idx = 1; idx < argc; ++idx ) {
		if( !strcmp( argv[idx], "-level" ) ) {
		    heesch_level = atoi(argv[idx+1]);
		    ++idx;
		} else if( !strcmp( argv[idx], "-noreflections" ) ) {
		    no_reflections = true;
		} else if( !strcmp( argv[idx], "-extremes" ) ) {
		    extremes = true;
		} else {
			cerr << "Unrecognized parameter \"" << argv[idx] << "\""
				<< endl;
			exit( 0 );
		}
	}

	FOR_EACH_IN_STREAM( cin, computeSurrounds );
	return 0;
}
