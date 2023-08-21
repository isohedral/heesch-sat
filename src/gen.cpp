#include <iostream>
#include <sstream>

#include "redelmeier.h"
#include "grid.h"
#include "tileio.h"

using namespace std;

static bool onlyfree = false;
static bool units = false;
static bool holes = true;
static size_t numcells = 1;

template<typename grid>
static bool readShape( istream& is, Shape<grid>& shape )
{
	using coord_t = typename grid::coord_t;
	using iter = IntReader<coord_t>;

	char buf[1000];
	is.getline( buf, 1000 );
	size_t gc = is.gcount();
	if( gc == 0 ) {
		return false;
	}
	iter iend { buf + gc - 1 };

	for( auto i = iter { buf }; i != iend; ) {
		shape.add( *i++, *i++ );
	}

	if( shape.size() > 0 ) {
		shape.complete();
		return true;
	} else {
		return false;
	}
}

template<typename grid>
static vector<Shape<grid>> readShapes( istream& is )
{
	vector<Shape<grid>> ret;

	Shape<grid> cur;
	while( readShape( is, cur ) ) {
		// cur.debug();
		ret.push_back( std::move( cur ) );
	}

	return ret;
}

template<typename grid>
void output( const Shape<grid>& shp )
{
	static TileInfo<grid> info;
	info.setShape( shp );
	info.setRecordType( TileInfo<grid>::UNKNOWN );

	if( !shp.simplyConnected() ) {
		// Shape has a hole.  Report if argument is set, otherwise skip
		if( holes ) {
			info.setRecordType( TileInfo<grid>::HOLE );
			info.write( cout );
		}
	} else {
		info.write( cout );
	}
}

template<typename grid>
static void gridMain( int )
{
	polyform_cb<grid> cb { output<grid> };

	if( units ) {
		vector<Shape<grid>> shapes = readShapes<grid>( cin );
		RedelmeierCompound<grid> comp { shapes };

		if( onlyfree ) {
			CanonSortUniq<grid> filt {};
			filt.solve( numcells, comp, cb );
		} else {
			comp.solve( numcells, cb );
		}
	} else {
		RedelmeierSimple<grid> simp;
		if( onlyfree ) {
			FreeFilter<grid> filt {};
			filt.solve( numcells, simp, cb );
		} else {
			simp.solve( numcells, cb );
		}
	}
}
GRID_WRAP( gridMain );

int main( int argc, char **argv )
{
	GridType gt = getGridType( argc, argv );

	size_t idx = 1;

	while( idx < argc ) {
		if( !strcmp( argv[idx], "-size" ) ) {
			numcells = atoi( argv[idx+1] );
			++idx;
		} else if( !strcmp( argv[idx], "-free" ) ) {
			onlyfree = true;
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

	GRID_DISPATCH( gridMain, gt, 0 );
	return 0;
}
