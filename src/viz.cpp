#include <iostream>
#include <fstream>

#include <cairo.h>
#include <cairo-pdf.h>

#include "visualizer.h"
#include "grid.h"
#include "tileio.h"

// Visualize a set of polyforms and their properties in a PDF file.

static bool ori_col = false;
bool shapes_only = false;

static cairo_surface_t *pdf;
static cairo_t *cr;

// Draw 6x8 grids of shapes per page.
static size_t grid_y = 0;
static size_t grid_x = 0;

static bool nodraw = false;

static bool draw_all = true;
static bool draw_unknown = false;
static bool draw_holes = false;
static bool draw_inconclusive = false;
static bool draw_nontiler = false;
static bool draw_isohedral = false;
static bool draw_all_heesch = true;
static std::unordered_set<size_t> hcs;
static std::unordered_set<size_t> hhs;

using namespace std;

static ofstream extract;

template<typename grid>
static bool shouldDraw( const TileInfo<grid>& tile )
{
	using info_t = TileInfo<grid>;
	switch( tile.getRecordType() ) {
		case info_t::UNKNOWN:
			return draw_all || draw_unknown;
		case info_t::HOLE:
			return draw_all || draw_holes;
		case info_t::INCONCLUSIVE:
			return draw_all || draw_inconclusive;
		case info_t::NONTILER:
			if( draw_all || draw_nontiler ) {
				if( draw_all_heesch ) {
					return true;
				}
				if( hcs.find( tile.getHeeschConnected() ) != hcs.end() ) {
					return true;
				}
				if( hhs.find( tile.getHeeschHoles() ) != hhs.end() ) {
					return true;
				}
			}
			return false;
		case info_t::ISOHEDRAL:
			return draw_all || draw_isohedral;
		default:
			return draw_all;
		}
}

template<typename grid>
static bool drawPatch( const TileInfo<grid>& tile )
{
/*
	std::cerr << "Working on ";
	tile.getShape().debug();
	*/

	if( !shouldDraw( tile ) ) {
		return true;
	}

	if( extract.is_open() ) {
		tile.write( extract );
	}

	if( nodraw ) {
		return true;
	}

    Visualizer<grid> viz { cr, tile };
	viz.setColourByOrientation( ori_col );

	double sc = std::min( (6.5*72.0) / 500.0, (9.0*72.0) / 500.0 );

	cairo_save( cr );
	cairo_translate( cr, 4.25*72, (11-4.25)*72 );
	cairo_scale( cr, sc, sc );
	cairo_translate( cr, -250.0, -250.0 );

	if( tile.getRecordType() == TileInfo<grid>::NONTILER ) {
		viz.drawNontiler();
	} else if( tile.getRecordType() == TileInfo<grid>::INCONCLUSIVE ) {
		viz.drawInconclusive();
	} else {
		viz.drawShape( true );
	}

	cairo_restore( cr );

	if( tile.getRecordType() == TileInfo<grid>::NONTILER ) {
		cairo_save( cr );
		cairo_translate( cr, 6.5*72, 72 );
		cairo_scale( cr, 72.0 / 500.0, 72.0 / 500.0 );
		viz.drawShape( true );
		cairo_restore( cr );
	}

	viz.drawText( 72, 72 );

	cairo_surface_show_page( pdf );

	cairo_status_t status = cairo_status( cr );
	if( status ) {
		cerr << "Error while processing ";
		tile.getShape().debug();
		cerr << cairo_status_to_string( status ) << endl;
	}

	return true;
}
GRID_WRAP( drawPatch );

template<typename grid>
static bool drawShapes( const TileInfo<grid>& tile )
{
    Visualizer<grid> viz { cr, tile };

	double sc = std::min( (7.5*72.0) / (6*500.0), (9.0*72.0) / (8*500.0) );

	cairo_save( cr );
	cairo_translate( cr, 4.25*72, 5.5*72 );
	cairo_scale( cr, sc, sc );
	cairo_translate( cr, -6*250.0, -8*250.0 );
	cairo_translate( cr, grid_x * 500.0, grid_y * 500.0 );

	viz.drawShape( true );

	cairo_restore( cr );

	++grid_x;
	if( grid_x == 6 ) {
		grid_x = 0;
		++grid_y;
		if( grid_y == 8 ) {
			grid_y = 0;
			cairo_surface_show_page( pdf );
		}
	}


	return true;
}
GRID_WRAP( drawShapes );

int main( int argc, char **argv )
{
	const char *inname = nullptr;
	const char *outname = "out.pdf";

    for (int idx = 1; idx < argc; ++idx) {
        if( !strcmp(argv[idx], "-orientation") ) {
			ori_col = true;
        } else if( !strcmp(argv[idx], "-shapes") ) {
			shapes_only = true;
		} else if( !strcmp( argv[idx], "-o" ) ) {
			++idx;
			outname = argv[idx];
		} else if( !strcmp( argv[idx], "-nodraw" ) ) {
			nodraw = true;
		} else if( !strcmp( argv[idx], "-e" ) ) {
			// Extract selected tile records to a separate file
			++idx;
			extract.open( argv[idx] );
		} else if( !strcmp( argv[idx], "-unknown" ) ) {
			draw_unknown = true;
		} else if( !strcmp( argv[idx], "-holes" ) ) {
			draw_holes = true;
		} else if( !strcmp( argv[idx], "-inconclusive" ) ) {
			draw_inconclusive = true;
		} else if( !strcmp( argv[idx], "-nontiler" ) ) {
			draw_nontiler = true;
		} else if( !strcmp( argv[idx], "-isohedral" ) ) {
			draw_isohedral = true;
		} else if( !strcmp( argv[idx], "-hcs" ) ) {
			draw_all_heesch = false;
			++idx;
			IntReader i { argv[idx] };
			IntReader iend { argv[idx] + strlen( argv[idx] ) };
			while( i != iend ) {
				hcs.insert( *i );
				++i;
			}
		} else if( !strcmp( argv[idx], "-hhs" ) ) {
			draw_all_heesch = false;
			++idx;
			IntReader i { argv[idx] };
			IntReader iend { argv[idx] + strlen( argv[idx] ) };
			while( i != iend ) {
				hhs.insert( *i );
				++i;
			}
		} else {
			// Maybe an input filename?
			if( filesystem::exists( argv[idx] ) ) {
				inname = argv[idx];
			} else {
				cerr << "Argument \"" << argv[idx] 
					<< "\" is neither a file name nor a valid parameter"
					<< endl;
				exit( 0 );
			}
		}
    }

	if( draw_unknown || draw_holes || draw_inconclusive || draw_nontiler
			|| draw_isohedral ) {
		draw_all = false;
	}

	if( !nodraw ) {
		pdf = cairo_pdf_surface_create( outname, 8.5*72, 11*72 );
		cr = cairo_create( pdf );
	}

	if( shapes_only ) {
		if( inname ) {
			ifstream ifs( inname );
			FOR_EACH_IN_STREAM( ifs, drawShapes );
		} else {
			FOR_EACH_IN_STREAM( cin, drawShapes );
		}

		if( !((grid_x == 0) && (grid_y == 0)) && !nodraw ) {
			cairo_surface_show_page( pdf );
		}
	} else {
		if( inname ) {
			ifstream ifs( inname );
			FOR_EACH_IN_STREAM( ifs, drawPatch );
		} else {
			FOR_EACH_IN_STREAM( cin, drawPatch );
		}
	}

	if( !nodraw ) {
		cairo_status_t status = cairo_status( cr );
		if( status ) {
			cerr << cairo_status_to_string( status ) << endl;
		}
		cairo_destroy( cr );
		cairo_surface_finish( pdf );
		status = cairo_surface_status( pdf );
		if( status ) {
			cerr << cairo_status_to_string( status ) << endl;
		}
		cairo_surface_destroy( pdf );
	}

	if( extract.is_open() ) {
		extract.flush();
		extract.close();
	}

	return 0;
}
