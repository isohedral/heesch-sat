#include <iostream>
#include <cstdint>
#include <sstream>

#include "heesch.h"
#include "grid.h"
#include "tileio.h"

using namespace std;

static bool no_reflections = false;
static bool extremes = false;
static size_t heesch_level = 1;

template<typename grid>
static bool readShape( istream& is, Shape<grid>& shape, string& str )
{
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;

	char buf[1000];
	is.getline( buf, 1000 );
	str = buf;

	shape.reset();
	istringstream iss( buf );

	coord_t x;
	coord_t y;

	while( iss >> x >> y ) {
		shape.add( point_t { x, y } );
	}

	shape.complete();
	return shape.size() > 0;
}

template<typename grid>
static void mainLoop( istream& is )
{
	using coord_t = typename grid::coord_t;

	Shape<grid> shape;
	string desc;

	while( readShape( is, shape, desc ) ) {
		TileInfo<grid> info;
		info.setShape( shape );

		HeeschSolver<grid> solver { shape, no_reflections ? TRANSLATIONS_ROTATIONS : ALL };
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
	}
}

template<typename grid>
static void gridMain( int argc, char **argv )
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

	mainLoop<grid>( cin );
}
GRID_WRAP( gridMain );

int main( int argc, char **argv )
{
	GridType gt = getGridType( argc, argv );
	GRID_DISPATCH( gridMain, gt, argc, argv );
	return 0;
}
