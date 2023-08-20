#include <iostream>
#include <cstdint>
#include <sstream>

#include "heesch.h"
#include "grid.h"
#include "tileio.h"

using namespace std;

static bool show_solution = false;
// Ha ha, set to one more than the Heesch record, just in case.
static int max_level = 7;
static Orientations ori = ALL;
static bool check_isohedral = false;
static bool update_only = false;

size_t tilings = 0;

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
static bool computeHeesch( const TileInfo<grid>& tile )
{
	using coord_t = typename grid::coord_t;

	if( update_only ) {
		// If we're updating, we only want to deal with unknown or 
		// inconclusive records.
		if( !((tile.getRecordType() == TileInfo<grid>::UNKNOWN) 
				|| (tile.getRecordType() == TileInfo<grid>::INCONCLUSIVE)) ) {
			tile.write( cout );
			return true;
		}
	}

	if( tile.getRecordType() == TileInfo<grid>::HOLE ) {
		// Don't compute heesch number of something with a hole
		tile.write( cout );
		return true;
	}

	TileInfo<grid> work { tile };

	size_t hc = 0;
	Solution<coord_t> sc;
	size_t hh = 0;
	Solution<coord_t> sh;
	bool has_holes;

	HeeschSolver<grid> solver { work.getShape(), ori };
	solver.setCheckIsohedral( check_isohedral );
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
		} if( solver.tilesIsohedrally() ) {
			work.setPeriodic( 1 );
			work.write( cout );
			return true;
		}
	}

	if( solver.getLevel() > max_level ) {
		// Exceeded maximum level, label it inconclusive
		work.setRecordType( TileInfo<grid>::INCONCLUSIVE );
	} else if( show_solution ) {
		work.setNonTiler( hc, &sc, hh, &sh );
	} else {
		work.setNonTiler( hc, nullptr, hh, nullptr );
	}
	work.write( cout );
	return true;
}
GRID_WRAP( computeHeesch );

int main( int argc, char **argv )
{
	// bootstrap_grid( argc, argv, gridMain ) 
	// GridType gt = getGridType( argc, argv );

	for( size_t idx = 1; idx < argc; ++idx ) {
		if( !strcmp( argv[idx], "-show" ) ) {
			show_solution = true;
		} else if( !strcmp( argv[idx], "-maxlevel" ) ) {
		    max_level = atoi(argv[idx+1]);
		    ++idx;
		} else if( !strcmp( argv[idx], "-translations" ) ) {
			ori = TRANSLATIONS_ONLY;
		} else if( !strcmp( argv[idx], "-rotations" ) ) {
			ori = TRANSLATIONS_ROTATIONS;
		} else if( !strcmp( argv[idx], "-isohedral" ) ) {
			check_isohedral = true;
		} else if( !strcmp( argv[idx], "-update" ) ) {
			update_only = true;
		} else {
			cerr << "Unrecognized parameter \"" << argv[idx] << "\""
				<< endl;
			exit( 0 );
		}
	}

	FOR_EACH_IN_STREAM( cin, computeHeesch );
	return 0;
}
