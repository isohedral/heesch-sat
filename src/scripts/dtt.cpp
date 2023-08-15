#include <iostream>

#include "draftergrid.h"

using namespace std;

int main( void )
{
	using coord = int16_t;
	using grid = DrafterGrid<coord>;
	using point_t = typename grid::point_t;

	for( short y = 0; y < 7; ++y ) {
		for( short x = 0; x < 7; ++x ) {
			grid::TileType tt = grid::getTileType( point_t { x, y } );
			if( tt == grid::INVALID ) {
				cout << "INVALID," << endl;
			} else {
				cout << "TRIANGLE_" << tt << "," << endl;
			}
				
		}
	}

	return 0;
}
