#include <iostream>

#include <cairo.h>
#include <cairo-pdf.h>

#include "visualizer.h"
#include "grid.h"
#include "tileio.h"

static bool ori_col = false;

static cairo_surface_t *pdf;
static cairo_t *cr;

using namespace std;

template<typename grid>
static bool drawPatch( const TileInfo<grid>& tile )
{
    Visualizer<grid> viz { cr, tile };

	double sc = std::min( 500.0 / (7.5*72.0), 500.0 / (9.0*72.0) );

	cairo_save( cr );
	cairo_translate( cr, 4.25*72, 5.5*72 );
	cairo_scale( cr, sc, sc );
	cairo_translate( cr, -250.0, -250.0 );

	if( tile.getRecordType() == TileInfo<grid>::NONTILER ) {
		viz.drawPatch();
	} else {
		viz.drawShape();
	}

	cairo_restore( cr );
	cairo_surface_show_page( pdf );

	return true;
}
GRID_WRAP( drawPatch );

int main( int argc, char **argv )
{
	const char *outname = "out.pdf";

    for (int idx = 1; idx < argc; ++idx) {
        if( !strcmp(argv[idx], "-orientation") ) {
			ori_col = true;
		} else if( !strcmp( argv[idx], "-o" ) ) {
			++idx;
			outname = argv[idx];
		} else {
			cerr << "Unrecognized parameter \"" << argv[idx] << "\""
				<< endl;
			exit( 0 );
		}
    }

	pdf = cairo_pdf_surface_create( outname, 8.5*72, 11*72 );
	cr = cairo_create( pdf );

	FOR_EACH_IN_STREAM( cin, drawPatch );

	cairo_destroy( cr );
	cairo_surface_finish( pdf );
	cairo_surface_destroy( pdf );

	return 0;
}
