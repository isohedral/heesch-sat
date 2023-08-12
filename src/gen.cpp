#include <iostream>

#include "redelmeier.h"
#include "grid.h"

using namespace std;

template<typename grid>
static void output( const typename grid::point_t&,
	const std::vector<typename grid::point_t>& pts )
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
	bool free = false;

	for( size_t idx = 1; idx < argc; ++idx ) {
		if( !strcmp( argv[idx], "-free" ) ) {
			free = true;
		}
	}

	polyform_cb<grid> cb { output<grid> };

	size_t size = 1;
	size_t idx = 1;
	while( idx < argc ) {
		if( !strcmp( argv[idx], "-size" ) ) {
			size = atoi( argv[idx+1] );
			++idx;
		}
		++idx;
	}

	if( free ) {
    	FreeFilter<grid> r {};
		r.solve( size, cb );
	} else {
		RedelmeierSimple<grid> r {};
		r.solve( size, cb );
	}
}

int main( int argc, char **argv )
{
	bootstrap_grid( argc, argv, gridMain ) 
	return 0;
}
