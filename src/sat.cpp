#include <iostream>
#include <cstdint>
#include <fstream>
#include <filesystem>

#include "heesch.h"
#include "grid.h"
#include "tileio.h"

// Use a SAT solver to compute Heesch numbers of polyforms.

using namespace std;

static bool show_solution = false;
// Ha ha, set to one more than the Heesch record, just in case.
static size_t max_level = 7;
static Orientations ori = ALL;
static bool check_hh = false;
static bool reduce = true;
static bool failsafe = false;
static bool check_isohedral = false;
static bool check_periodic = false;
static bool update_only = false;

static const char *inname = nullptr;
static const char *outname = nullptr;
static ofstream ofs;
static ostream *out;

template<typename grid>
static bool computeHeesch(TileInfo<grid>& info)
{
	if( update_only ) {
		// If we're updating, we only want to deal with unknown or 
		// inconclusive records.
		if( !((info.getRecordType() == TileInfo<grid>::UNKNOWN) 
				|| (info.getRecordType() == TileInfo<grid>::INCONCLUSIVE)) ) {
			info.write( *out );
			return true;
		}
	}

	if( info.getRecordType() == TileInfo<grid>::HOLE ) {
		// Don't compute heesch number of something with a hole
		info.write( *out );
		return true;
	}

	HeeschSolver<grid> solver {info.getShape(), ori, reduce};
	solver.setCheckIsohedral(check_isohedral);
	solver.setCheckPeriodic(check_periodic);
	solver.setCheckHoleCoronas(check_hh);
	solver.solve(show_solution, max_level, info);
	info.write(*out);

	return true;
}

// An older implementation that can be used as a reference for
// testing optimizations, for example.
template<typename grid>
static bool computeHeeschSafeMode( TileInfo<grid>& tile )
{
	using coord_t = typename grid::coord_t;

	if (update_only) {
		// If we're updating, we only want to deal with unknown or 
		// inconclusive records.
		if (!((tile.getRecordType() == TileInfo<grid>::UNKNOWN) 
				|| (tile.getRecordType() == TileInfo<grid>::INCONCLUSIVE))) {
			tile.write(*out);
			return true;
		}
	}

	if (tile.getRecordType() == TileInfo<grid>::HOLE) {
		// Don't compute heesch number of something with a hole
		tile.write(*out);
		return true;
	}

	size_t hc = 0;
	LabelledPatch<coord_t> sc;
	size_t hh = 0;
	LabelledPatch<coord_t> sh;
	bool has_holes;

	HeeschSolver<grid> solver {tile.getShape(), ori, reduce};
	solver.setCheckIsohedral(check_isohedral);
	solver.setCheckHoleCoronas(check_hh);

	LabelledPatch<coord_t> cur;

	if (solver.isSurroundable()) {
		solver.increaseLevel();

		while (true) {
			if (solver.getLevel() > max_level) {
				break;
			}

			if (solver.hasCorona(show_solution, has_holes, cur)) {
				if (has_holes) {
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
			} else if (solver.tilesIsohedrally()) {
				tile.setPeriodic(1);
				tile.write(*out);
				return true;
			} else {
				break;
			}
		}
	}

	if (solver.getLevel() > max_level) {
		// Exceeded maximum level, label it inconclusive
		if (show_solution) {
			tile.setInconclusive(cur);
		} else {
			tile.setInconclusive();
		}
	} else if (show_solution) {
		tile.setNonTiler(hc, &sc, hh, &sh);
	} else {
		tile.setNonTiler(hc, nullptr, hh, nullptr);
	}
	tile.write(*out);
	return true;
}

int main( int argc, char **argv )
{
	ios_base::sync_with_stdio(false);
	cin.tie(nullptr);

	for (int idx = 1; idx < argc; ++idx) {
		if (!strcmp(argv[idx], "-show")) {
			show_solution = true;
		} else if (!strcmp(argv[idx], "-o")) {
			++idx;
			outname = argv[idx];
		} else if (!strcmp(argv[idx], "-maxlevel")) {
		    ++idx;
		    max_level = atoi(argv[idx]);
		} else if (!strcmp(argv[idx], "-translations")) {
			ori = TRANSLATIONS_ONLY;
		} else if (!strcmp(argv[idx], "-rotations")) {
			ori = TRANSLATIONS_ROTATIONS;
		} else if (!strcmp(argv[idx], "-isohedral")) {
			check_isohedral = true;
		} else if (!strcmp(argv[idx], "-periodic")) {
			check_periodic = true;
		} else if (!strcmp(argv[idx], "-noisohedral")) {
			check_isohedral = false;
		} else if (!strcmp(argv[idx], "-update")) {
			update_only = true;
		} else if (!strcmp(argv[idx], "-hh")) {
			check_hh = true;
		} else if (!strcmp(argv[idx], "-reduce")) {
			reduce = true;
		} else if (!strcmp(argv[idx], "-noreduce")) {
			reduce = false;
		} else if (!strcmp(argv[idx], "-old")) {
			failsafe = true;
		} else if (!strcmp(argv[idx], "-new")) {
			failsafe = false;
		} else {
			// Maybe an input filename?
			if (filesystem::exists(argv[idx])) {
				inname = argv[idx];
			} else {
				cerr << "Argument \"" << argv[idx] 
					<< "\" is neither a file name nor a valid parameter"
					<< endl;
				exit(0);
			}
		}
		// NOTE: Consider adding a -paranoid flag that incorporates
		// testing cross-checks (e.g., boundary-based isohedral),
		// but leaves those checks off by default.
	}

	if (outname) {
		ofs.open(outname);
		out = &ofs;
	} else {
		out = &cout;
	}

	if (inname) {
		ifstream ifs(inname);
		if (failsafe) {
			processInputStream(ifs, computeHeeschSafeMode);
		} else {
			processInputStream(ifs, computeHeesch);
		}
	} else {
		if (failsafe) {
			processInputStream(cin, computeHeeschSafeMode);
		} else {
			processInputStream(cin, computeHeesch);
		}
	}

	if (ofs.is_open()) {
		ofs.flush();
		ofs.close();
	}

	return 0;
}
