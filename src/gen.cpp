#include <iostream>

#include "redelmeier.h"
#include "grid.h"

using namespace std;

template<typename grid>
static void output( const vector<typename grid::point_t>& pts )
{
	bool at_start = true;

	for( const auto& p : pts ) {
		if( !at_start ) {
			cout << ' ';
		}
		cout << p.x_ << ' ' << p.y_;
		at_start = false;
	}
	cout << endl;
}

template<typename grid>
static void gridMain( int argc, char **argv )
{
	size_t size = 1;
	size_t idx = 1;
	while( idx < argc ) {
		if( !strcmp( argv[idx], "-size" ) ) {
			size = atoi( argv[idx+1] );
			++idx;
		}
		++idx;
	}

    RedelmeierSimple<grid> r { output<grid>, size };
	r.solve();
}

int main( int argc, char **argv )
{
	bootstrap_grid( argc, argv, gridMain ) 
	return 0;
}
