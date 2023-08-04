#include <iostream>

#include "visualizer.h"

// #include "octasquaregrid.h"
// #include "grid3636.h"
// #include "abologrid.h"
// #include "draftergrid.h"
// #include "kitegrid.h"
#include "ominogrid.h"

/*
enum GRID {
    OCTASQUARE,
    GRID3636,
    ABOLO,
    DRAFTER,
	KITE
};
*/

enum GRID {
	OMINO
};

static GRID grid_type = OMINO;
static int max_level = INT_MAX;
static bool max_only = false;

static bool ori_col = false;

template<typename grid>
static void mainLoop(std::istream& in) {
    Visualizer<grid> p{std::cin, std::cout, false, max_level, max_only, ori_col};
    p.run();
}

int main(int argc, char **argv) {

    for (int idx = 1; idx < argc; ++idx) {
        if( !strcmp( argv[idx], "-maxlevel" ) ) {
            // Display the shapes whose Heesch number is maximum
            // among those with Heesch number <= max_level-1
            max_level = atoi(argv[idx+1]);
            ++idx;
		} else if (!strcmp(argv[idx], "-maxonly")) {
			max_only = true;
			/*
        } else if (!strcmp(argv[idx], "-3636")) {
            grid_type = GRID3636;
        } else if (!strcmp(argv[idx], "-abolo")) {
            grid_type = ABOLO;
        } else if (!strcmp(argv[idx], "-drafter")) {
            grid_type = DRAFTER;
        } else if (!strcmp(argv[idx], "-kite")) {
            grid_type = KITE;
			*/
        } else if (!strcmp(argv[idx], "-orientation")) {
			ori_col = true;
		}
    }

/*
    if (grid_type == OCTASQUARE) {
        mainLoop<OctaSquareGrid<int16_t>>(std::cin);
    } else if (grid_type == GRID3636) {
        mainLoop<Grid3636<int16_t>>(std::cin);
    } else if (grid_type == ABOLO) {
        mainLoop<AboloGrid<int16_t>>(std::cin);
    } else if (grid_type == DRAFTER) {
        mainLoop<DrafterGrid<int16_t>>(std::cin);
    } else if (grid_type == KITE) {
        mainLoop<KiteGrid<int16_t>>(std::cin);
	}

*/

	mainLoop<OminoGrid<int16_t>>(std::cin);

    return 0;
}
