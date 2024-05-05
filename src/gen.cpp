#include <iostream>
#include <fstream>

#include "redelmeier.h"
#include "grid.h"
#include "tileio.h"

// Generate polyforms using Redelmeier's algorithm.

using namespace std;

static bool onlyfree = false;
static bool units = false;
static bool holes = false;
static size_t numcells = 0;
static vector<size_t> sizes;

const char *outname = nullptr;

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
class Outputter
{
public:
	Outputter( ostream& out )
		: out_ { out }
	{}

	void operator()( const Shape<grid>& shp )
	{
		static TileInfo<grid> info;
		info.setShape( shp );
		info.setRecordType( TileInfo<grid>::UNKNOWN );

		if( !shp.simplyConnected() ) {
			// Shape has a hole.  Report if argument is set, otherwise skip
			if( holes ) {
				info.setRecordType( TileInfo<grid>::HOLE );
				info.write( out_ );
			}
		} else {
			info.write( out_ );
		}
	}
	
private:
	ostream& out_;
};

template<typename grid>
static void gridMain( int )
{
	ofstream ofs;

	if( outname ) {
		ofs.open( outname );
	}

	polyform_cb<grid> cb { Outputter<grid>( outname ? ofs : cout ) };

	if( units ) {
		if( numcells == 0 ) {
			cerr << "Error: Must provide size for units generation" << endl;
			exit( 0 );
		}

		vector<Shape<grid>> shapes = readShapes<grid>( cin );
		RedelmeierCompound<grid> comp { shapes };

		if( onlyfree ) {
			CanonSortUniq<grid> filt {};
			filt.solve( numcells, comp, cb );
		} else {
			comp.solve( numcells, cb );
		}
	} else {
		if( numcells == 0 ) {
			if( sizes.size() != grid::num_tile_shapes ) {
				cerr << "Error: Incorrect number of sizes" << endl;
				exit( 0 );
			}

			RedelmeierSimple<grid> simp;
			if( onlyfree ) {
				FreeFilter<grid> filt {};
				filt.solve( sizes, simp, cb );
			} else {
				simp.solve( sizes, cb );
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

	if( outname ) {
		ofs.flush();
		ofs.close();
	}
}
GRID_WRAP( gridMain );

int main( int argc, char **argv )
{
	ios_base::sync_with_stdio( false );
	cin.tie( nullptr );

	GridType gt = getGridType( argc, argv );

	size_t idx = 1;

	while( idx < argc ) {
		if( !strcmp( argv[idx], "-size" ) ) {
			numcells = atoi( argv[idx+1] );
			++idx;
		} else if( !strcmp( argv[idx], "-sizes" ) ) {
			++idx;
			IntReader i { argv[idx] };
			IntReader iend { argv[idx] + strlen( argv[idx] ) };
			while( i != iend ) {
				sizes.push_back( *i );
				++i;
			}
		} else if( !strcmp( argv[idx], "-o" ) ) {
			++idx;
			outname = argv[idx];
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
