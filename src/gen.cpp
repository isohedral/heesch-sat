#include <iostream>
#include <sstream>

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
static bool readShape( istream& is, Shape<grid>& shape )
{
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;

	char buf[1000];
	is.getline( buf, 1000 );
	string str = buf;

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

template<typename grid>
static vector<Shape<grid>> readShapes( istream& is )
{
	vector<Shape<grid>> ret;

	Shape<grid> cur;
	while( readShape( is, cur ) ) {
		ret.push_back( cur );
	}

	return ret;
}

template<typename grid>
static void gridMain( int argc, char **argv )
{
	polyform_cb<grid> cb { output<grid> };

	bool free = false;
	bool units = false;
	bool holes = false;
	size_t size = 1;
	size_t idx = 1;

	while( idx < argc ) {
		if( !strcmp( argv[idx], "-size" ) ) {
			size = atoi( argv[idx+1] );
			++idx;
		} else if( !strcmp( argv[idx], "-free" ) ) {
			free = true;
		} else if( !strcmp( argv[idx], "-units" ) ) {
			units = true;
		} else if( !strcmp( argv[idx], "-holes" ) ) {
			holes = true;
		} else {
			cerr << "Unrecognized parameter \"" << argv[idx] << "\""
				<< endl;
			exit( 0 );
		}
		++idx;
	}

/*
	cerr << "units: " << units << endl;
	cerr << "free: " << free << endl;
	cerr << "size: " << size << endl;
*/

	if( units ) {
		vector<Shape<grid>> shapes = readShapes<grid>( cin );
		RedelmeierCompound<grid> r { shapes, holes };
		r.solve( size, cb );
	} else {
		RedelmeierSimple<grid> simp { holes };
		if( free ) {
			FreeFilter<grid> filt {};
			filt.solve( size, simp, cb );
		} else {
			simp.solve( size, cb );
		}
	}
}

int main( int argc, char **argv )
{
	bootstrap_grid( argc, argv, gridMain ) 
	return 0;
}
