// This is a test program for periodic tiling ideas.  It's currently
// superseded by HeeschSolver::checkIsohedralTiling

#include <iostream>
#include <cstdint>
#include <sstream>

#include <cryptominisat.h>

#include "grid.h"
#include "cloud.h"

using namespace std;

template<typename grid>
static bool readShape( istream& is, Shape<grid>& shape, string& str )
{
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;

	char buf[1000];
	is.getline( buf, 1000 );
	str = buf;

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
void populate( 
	const Shape<grid>& shape, const typename grid::xform_t& T, 
	point_set<typename grid::coord_t>& ps )
{
	for( const auto& p : shape ) {
		ps.insert( T * p );
	}
}

template<typename grid>
static void debugPatch( 
	const Shape<grid>& shape, const typename grid::xform_t *ts, size_t sz )
{
	for( const auto& p : shape ) {
		cout << p.x_ << ' ' << p.y_ << ' ';
	}
	cout << endl;
	cout << "Hc = 1 Hh = 1" << endl;
	cout << (sz+1) << endl;
	cout << "0 ; <1,0,0,0,1,0>" << endl;
	for( size_t idx = 0; idx < sz; ++idx ) {
		cout << "1 ; " << ts[idx] << endl;
	}
}

template<typename grid>
static bool tilesByTranslation( const Shape<grid>& shape )
{
	// shape.debug();

	Cloud<grid> cloud( shape );

	using coord_t = typename grid::coord_t;
	using xform_t = typename grid::xform_t;

	xform_map<coord_t, xform_t> tpairs;

	for( const auto& T : cloud.adjacent_ ) {
// 		if( T.isTranslation() ) {
			xform_t Ti = T.invert();
			if( cloud.isAdjacent( Ti ) ) {
				if( tpairs.find( Ti ) == tpairs.end() ) {
					tpairs[ T ] = Ti;
				}
			}
//		}
	}

	point_set<coord_t> shape_set;

	for( const auto& p : shape ) {
		shape_set.insert( p );
	}

	for( auto i = tpairs.begin(); i != tpairs.end(); ++i ) {
		for( auto j = tpairs.begin(); j != i; ++j ) {
			xform_t T2;
			if( cloud.isAdjacent( j->first * i->second ) ) {
				T2 = j->first;
			} else if( cloud.isAdjacent( j->second * i->second ) ) {
				T2 = j->second;
			} else {
				continue;
			}

			// T1 and T2 are both adjacent to the kernel.  Are they
			// adjacent to each other?
			if( !cloud.isAdjacent( T2 * i->second ) ) {
				continue;
			}
				
			xform_t nts[] = {
				i->first, i->second, j->first, j->second, 
				i->first * j->second, j->first * i->second,
				i->first * j->first, j->second * i->second };
			bool halo_ok = true;

			for( const auto& p : cloud.halo_ ) {
				bool found = false;

				for( size_t idx = 0; idx < 8; ++idx ) {
					if( shape_set.find( nts[idx] * p ) != shape_set.end() ) {
						found = true;
						break;
					}
				}

				if( !found ) {
					halo_ok = false;
					break;
				}
			}

			if( !halo_ok ) {
				debugPatch( shape, nts, 8 );
				continue;
			}

			// cout << "Got it! " << T1 << " / " << T2 << endl;

			return true;
		}
	}

/*
	shape.debug();
	for( const auto& p : tpairs ) {
		cout << "  " << p.first << " / " << p.second << endl;
	}
*/

	return false;
}

template<typename grid>
static void mainLoop( istream& is )
{
	size_t total = 0;
	size_t tiles = 0;

	Shape<grid> shape;
	string desc;

	while( readShape( is, shape, desc ) ) {
		++total;
		if( tilesByTranslation( shape ) ) {
			++tiles;
		}
	}

	cout << tiles << " of " << total << " tile by translation." << endl;
}

template<typename grid>
static void gridMain( int argc, char **argv )
{
	mainLoop<grid>( cin );
}

int main( int argc, char **argv )
{
	bootstrap_grid( argc, argv, gridMain ) 
	return 0;
}
