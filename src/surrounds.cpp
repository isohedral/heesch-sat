#include <iostream>
#include <cstdint>
#include <sstream>

#include "heesch.h"
#include "grid.h"

using namespace std;

static bool no_reflections = false;
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

template<typename coord>
static void reportConfig( ostream& os, const Solution<coord>& soln )
{
	if( !soln.empty() ) {
		os << soln.size() << endl;
		for( auto& pr : soln ) {
			os << pr.first << " ; " << pr.second << endl;
		}
	} else {
		os << "1" << endl << "0; <1,0,0,0,1,0>" << endl;
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

	readShape( is, shape, desc );

	HeeschSolver<grid> solver { shape, no_reflections ? TRANSLATIONS_ROTATIONS : ALL };
	for( size_t idx = 0; idx < heesch_level; ++idx ) {
		solver.increaseLevel();
	}

	std::vector<Solution<coord_t>> cur;
	solver.allCoronas( cur );
	for( const auto& soln : cur ) {
		report<coord_t,grid>( cout, desc, 1, soln, 1, soln );
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
		}
	}

	mainLoop<grid>( cin );
}

int main( int argc, char **argv )
{
	bootstrap_grid( argc, argv, gridMain ) 
	return 0;
}
