#include <iostream>
#include <cstdint>
#include <sstream>

#include "heesch.h"
#include "grid.h"

using namespace std;

static bool show_solution = false;
static int max_level = INT_MAX;
static bool reject_max = false;
static Orientations ori = ALL;

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
			os << "1" << endl << "0 ; <1,0,0,0,1,0>" << endl;
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

		HeeschSolver<grid> solver { shape, ori };
		solver.increaseLevel();

		while( true ) {
			// std::cerr << "Now at level " << solver.getLevel() << std::endl;
			// solver.debug( cout );
			Solution<coord_t> cur;

			if( solver.getLevel() > max_level ) {
				break;
			}

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

		if( (solver.getLevel() > max_level) && reject_max ) {
			continue;
		}
	
		report<coord_t,grid>( cout, desc, hc, sc, hh, sh );
	}
}

template<typename grid>
static void gridMain( int argc, char **argv )
{
	for( size_t idx = 1; idx < argc; ++idx ) {
		if( !strcmp( argv[idx], "-show" ) ) {
			show_solution = true;
		} else if( !strcmp( argv[idx], "-maxlevel" ) ) {
		    max_level = atoi(argv[idx+1]);
		    ++idx;
		} else if( !strcmp( argv[idx], "-reject" ) ) {
			reject_max = true;
		} else if( !strcmp( argv[idx], "-translations" ) ) {
			ori = TRANSLATIONS_ONLY;
		} else if( !strcmp( argv[idx], "-rotations" ) ) {
			ori = TRANSLATIONS_ROTATIONS;
		} else {
			cerr << "Unrecognized parameter \"" << argv[idx] << "\""
				<< endl;
			exit( 0 );
		}
	}

	mainLoop<grid>( cin );
}

int main( int argc, char **argv )
{
	bootstrap_grid( argc, argv, gridMain ) 
	return 0;
}
