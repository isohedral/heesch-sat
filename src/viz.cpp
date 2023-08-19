#include <iostream>

#include "visualizer.h"
#include "grid.h"

static int max_level = INT_MAX;
static bool max_only = false;

static bool ori_col = false;

using namespace std;

template<typename grid>
static void gridMain( int argc, char **argv )
{
    for (int idx = 1; idx < argc; ++idx) {
        if( !strcmp( argv[idx], "-maxlevel" ) ) {
            // Display the shapes whose Heesch number is maximum
            // among those with Heesch number <= max_level-1
            max_level = atoi(argv[idx+1]);
            ++idx;
		} else if (!strcmp(argv[idx], "-maxonly")) {
			max_only = true;
        } else if (!strcmp(argv[idx], "-orientation")) {
			ori_col = true;
		} else {
			cerr << "Unrecognized parameter \"" << argv[idx] << "\""
				<< endl;
			exit( 0 );
		}
    }

    Visualizer<grid> p
		{std::cin, std::cout, false, max_level, max_only, ori_col};
    p.run();
}
GRID_WRAP( gridMain );

int main( int argc, char **argv )
{
	GridType gt = getGridType( argc, argv );
	GRID_DISPATCH( gridMain, gt, argc, argv );
	return 0;
}
