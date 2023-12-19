#include <iostream>
#include <fstream>
#include <map>

#include "tileio.h"

// Gather summary stats for the polyforms in an input stream.

using namespace std;

static size_t total = 0;
static size_t num_unknown = 0;
static size_t num_hole = 0;
static size_t num_inconclusive = 0;
static size_t num_nontiler = 0;
static size_t num_isohedral = 0;
static size_t num_anisohedral = 0;
static size_t num_aperiodic = 0;

static map<size_t,size_t> hcs;
static map<size_t,size_t> hhs;

template<typename grid>
static bool getShapeStats( const TileInfo<grid>& tile )
{
	using info_t = TileInfo<grid>;

	++total;

	switch( tile.getRecordType() ) {
		case info_t::UNKNOWN:
			++num_unknown;
			break;
		case info_t::HOLE:
			++num_hole;
			break;
		case info_t::INCONCLUSIVE:
			++num_inconclusive;
			break;
		case info_t::NONTILER: {
			++num_nontiler;
			size_t hc = tile.getHeeschConnected();
			size_t hh = tile.getHeeschHoles();
			/*
			size_t code = (hc*1000)+hh;
			++heesch_nums[ code ];
			*/
			++hcs[hc];
			++hhs[hh];
			break;
			}
		case info_t::ISOHEDRAL:
			++num_isohedral;
			break;
		case info_t::ANISOHEDRAL:
			++num_anisohedral;
			break;
		case info_t::APERIODIC:
			++num_aperiodic;
			break;
	}
	return true;
}
GRID_WRAP( getShapeStats );

static void printReport( ostream& os )
{
	os << "Total: " << total << " shapes" << endl;
	os << "  " << num_unknown << " unprocessed" << endl;
	os << "  " << num_hole << " with holes" << endl;
	os << "  " << num_inconclusive << " inconclusive" << endl;
	os << "  " << num_nontiler << " non-tilers" << endl;
	for( const auto& r : hcs ) {
		os << "    " << r.second << " with Hc = " << r.first << endl;
	}
	for( const auto& r : hhs ) {
		os << "    " << r.second << " with Hh = " << r.first << endl;
	}
	os << "  " << num_isohedral << " tile isohedrally" << endl;
	os << "  " << num_anisohedral << " tile anisohedrally" << endl;
	os << "  " << num_aperiodic << " tile aperiodically" << endl;
}

int main( int argc, char **argv )
{
	const char *inname = nullptr;
	const char *outname = nullptr;

    for (int idx = 1; idx < argc; ++idx) {
		if( !strcmp( argv[idx], "-o" ) ) {
			++idx;
			outname = argv[idx];
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

	if( inname ) {
		ifstream ifs { inname };
		FOR_EACH_IN_STREAM( ifs, getShapeStats );
	} else {
		FOR_EACH_IN_STREAM( cin, getShapeStats );
	}

	if( outname ) {
		ofstream ofs( outname );
		printReport( ofs );
		ofs.flush();
		ofs.close();
	} else {
		printReport( cout );
	}

	return 0;
}
