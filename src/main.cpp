#include <iostream>
#include <cstdint>
#include <sstream>

#include "heesch.h"

#include "ominogrid.h"
#include "hexgrid.h"
#include "iamondgrid.h"
#include "octasquaregrid.h"
#include "grid3636.h"
#include "abologrid.h"

using namespace std;

enum GRID {
	OMINO,
	HEX,
	IAMOND,
	OCTASQUARE,
	GRID3636,
	ABOLO
};

static bool show_solution = false;
static GRID grid_type = OMINO;

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

template<typename coord>
static void reportConfig( ostream& os, const Solution<coord>& soln )
{
	if( show_solution ) {
		if( !soln.empty() ) {
			os << soln.size() << endl;
			for( auto& pr : soln ) {
				os << pr.first << " ; " << pr.second << endl;
			}
		} else {
			os << "1" << endl << "0; <1,0,0,0,1,0>" << endl;
		}
	}
}

template<typename coord, typename grid>
static void report( ostream& os, const string& desc,
	size_t hc, const Solution<coord>& sc, 
	size_t hh, const Solution<coord>& sh )
{
	os << desc << endl;
	os << "Hc = " << hc << " Hh = " << hh << endl;
	reportConfig( os, sc );
	if( hc != hh ) {
		reportConfig( os, sh );
	}
}

template<typename grid>
static void mainLoop( istream& is )
{
	using coord_t = typename grid::coord_t;

	Shape<grid> shape;
	string desc;

	while( readShape( is, shape, desc ) ) {
		size_t hc = 0;
		Solution<coord_t> sc;
		size_t hh = 0;
		Solution<coord_t> sh;
		bool has_holes;

		HeeschSolver<grid> solver { shape };
		solver.increaseLevel();

		while( true ) {
			// solver.debug( cout );
			Solution<coord_t> cur;

			if( solver.hasCorona( show_solution, has_holes, cur ) ) {
				if( has_holes ) {
					sh = cur;
					hh = solver.getLevel();
					break;
				} else {
					hc = solver.getLevel();
					hh = hc;
					sh = cur;
					sc = sh;
					solver.increaseLevel();
				}
			} else {
				break;
			}
		}

		report<coord_t,grid>( cout, desc, hc, sc, hh, sh );
	}
}

int main( int argc, char **argv )
{
	for( size_t idx = 1; idx < argc; ++idx ) {
		if( !strcmp( argv[idx], "-show" ) ) {
			show_solution = true;
		} else if( !strcmp( argv[idx], "-omino" ) ) {
			grid_type = OMINO;
		} else if( !strcmp( argv[idx], "-hex" ) ) {
			grid_type = HEX;
		} else if( !strcmp( argv[idx], "-iamond" ) ) {
			grid_type = IAMOND;
		} else if( !strcmp( argv[idx], "-octasquare" ) ) {
            grid_type = OCTASQUARE;
        } else if( !strcmp( argv[idx], "-3636" ) ) {
            grid_type = GRID3636;
		} else if( !strcmp( argv[idx], "-abolo" ) ) {
		    grid_type = ABOLO;
		}
	}

	if( grid_type == OMINO ) {
		mainLoop<OminoGrid<int16_t>>( cin );
	} else if( grid_type == HEX ) {
		mainLoop<HexGrid<int16_t>>( cin );
	} else if( grid_type == IAMOND ) {
		mainLoop<IamondGrid<int16_t>>( cin );
	} else if( grid_type == OCTASQUARE ) {
        mainLoop<OctaSquareGrid<int16_t>>( cin );
    } else if( grid_type == GRID3636 ) {
        mainLoop<Grid3636<int16_t>>( cin );
	} else if( grid_type == ABOLO ) {
	    mainLoop<AboloGrid<int16_t>>( cin );
	}
}
