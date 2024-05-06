#include <iostream>
#include <cstdint>
#include <fstream>

#include "heesch.h"
#include "grid.h"
#include "tileio.h"

// Use a SAT solver to compute Heesch numbers of polyforms.

using namespace std;

static bool show_solution = false;
// Ha ha, set to one more than the Heesch record, just in case.
static int max_level = 7;
static Orientations ori = ALL;
static bool check_hh = false;
static bool reduce = false;
static bool check_isohedral = false;
static bool update_only = false;

static const char *inname = nullptr;
static const char *outname = nullptr;
static ofstream ofs;
static ostream *out;

int num_wins = 0;

template<typename grid>
static bool computeHeesch( const TileInfo<grid>& tile )
{
	using coord_t = typename grid::coord_t;

	if( update_only ) {
		// If we're updating, we only want to deal with unknown or 
		// inconclusive records.
		if( !((tile.getRecordType() == TileInfo<grid>::UNKNOWN) 
				|| (tile.getRecordType() == TileInfo<grid>::INCONCLUSIVE)) ) {
			tile.write( *out );
			return true;
		}
	}

	if( tile.getRecordType() == TileInfo<grid>::HOLE ) {
		// Don't compute heesch number of something with a hole
		tile.write( *out );
		return true;
	}

	TileInfo<grid> work { tile };

	size_t hc = 0;
	Solution<coord_t> sc;
	size_t hh = 0;
	Solution<coord_t> sh;
	bool has_holes;

	HeeschSolver<grid> solver { work.getShape(), ori, reduce };
	solver.setCheckIsohedral( check_isohedral );
	solver.setCheckHoleCoronas( check_hh );
	solver.increaseLevel();

	Solution<coord_t> cur;

	while( true ) {
		// std::cerr << "Now at level " << solver.getLevel() << std::endl;
		// solver.debug( *out );

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
		} else if( solver.tilesIsohedrally() ) {
			work.setPeriodic( 1 );
			work.write( *out );
			return true;
		} else {
			break;
		}
	}

	if( solver.getLevel() > max_level ) {
		// Exceeded maximum level, label it inconclusive
		if( show_solution ) {
			work.setInconclusive( &cur );
		} else {
			work.setInconclusive();
		}
	} else if( show_solution ) {
		work.setNonTiler( hc, &sc, hh, &sh );
	} else {
		work.setNonTiler( hc, nullptr, hh, nullptr );
	}
	work.write( *out );
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
		} else if( !strcmp( argv[idx], "-o" ) ) {
			++idx;
			outname = argv[idx];
		} else if( !strcmp( argv[idx], "-maxlevel" ) ) {
		    ++idx;
		    max_level = atoi(argv[idx]);
		} else if( !strcmp( argv[idx], "-translations" ) ) {
			ori = TRANSLATIONS_ONLY;
		} else if( !strcmp( argv[idx], "-rotations" ) ) {
			ori = TRANSLATIONS_ROTATIONS;
		} else if( !strcmp( argv[idx], "-isohedral" ) ) {
			check_isohedral = true;
		} else if( !strcmp( argv[idx], "-noisohedral" ) ) {
			check_isohedral = false;
		} else if( !strcmp( argv[idx], "-update" ) ) {
			update_only = true;
		} else if( !strcmp( argv[idx], "-hh" ) ) {
			check_hh = true;
		} else if( !strcmp( argv[idx], "-reduce" ) ) {
			reduce = true;
		} else if( !strcmp( argv[idx], "-noreduce" ) ) {
			reduce = false;
		} else {
			// Maybe an input filename?
			if( filesystem::exists( argv[idx] ) ) {
				inname = argv[idx];
			} else {
				cerr << "Argument \"" << argv[idx] 
					<< "\" is neither a file name nor a valid parameter"
					<< endl;
				exit( 0 );
			}
		}
	}

	if( outname ) {
		ofs.open( outname );
		out = &ofs;
	} else {
		out = &cout;
	}

	if( inname ) {
		ifstream ifs( inname );
		FOR_EACH_IN_STREAM( ifs, computeHeesch );
	} else {
		FOR_EACH_IN_STREAM( cin, computeHeesch );
	}

	if( ofs.is_open() ) {
		ofs.flush();
		ofs.close();
	}

	cerr << num_wins << " wins." << endl;
	return 0;
}
