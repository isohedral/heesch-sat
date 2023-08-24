#pragma once

#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>
#include <sstream>

#include <boost/functional/hash.hpp>

#include <cairo.h>

#include "geom.h"
#include "heesch.h"
#include "tileio.h"

template<typename coord>
using edge = std::pair<point<coord>,point<coord>>;
template<typename coord>
using edgeset = std::unordered_set<edge<coord>, boost::hash<edge<coord>>>;
using colour = std::array<double,3>;

template<typename grid>
class Visualizer
{
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;
	using xform_t = typename grid::xform_t;
	using patch_t = Solution<coord_t>;
	using edge_t = edge<coord_t>;
	using edgeset_t = edgeset<coord_t>;
	using info_t = TileInfo<grid>;

public:
	Visualizer( cairo_t *cr, const info_t& tile )
		: cr_ { cr }
		, tile_ { tile }
		, colour_by_orientation_ { false }
	{
		if( tile.getRecordType() != info_t::HOLE ) {
			initGridOutline();
		}
	}

	void setColourByOrientation( bool cbo )
	{
		colour_by_orientation_ = cbo;
	}

	void drawNontiler( bool just_hc = false ) const;
	void drawInconclusive() const;
	void drawShape( bool dashes = false ) const;
	void drawShapeCells( bool dashes = false ) const;

	void drawText( double x, double y ) const;

private:
	double centreToBounds( 
		double xmin, double xmax, double ymin, double ymax ) const
	{
		double sc = std::min( 450.0 / (xmax-xmin), 450.0 / (ymax-ymin) );
		cairo_translate( cr_, 250.0, 250.0 );
		cairo_scale( cr_, sc, sc );
		cairo_translate( cr_, -0.5*(xmin+xmax), -0.5*(ymin+ymax) );
		
		double lwx = 1.0;
		double lwy = 0.0;
		cairo_user_to_device_distance( cr_, &lwx, &lwy );
		double eff = 1.0 / sqrt( lwx * lwx + lwy * lwy );
		// cairo_set_line_width( cr_, lw * eff );
		return eff;
	}

	void drawTextLine( 
		std::ostringstream& out, double x, double& y, double h ) const
	{
		std::string str = std::move( out ).str();
		cairo_move_to( cr_, x, y );
		cairo_show_text( cr_, str.c_str() );

		out.str( "" );
		out.clear();
		y += h;
	}

	void drawPatch( const patch_t& patch, bool cbyo = false ) const;
	void drawPolygon( const std::vector<point<double>>& pts,
		double r, double g, double b, bool stroke = true ) const;

	edgeset_t getUniqueTileEdges() const;
	void initGridOutline();

	cairo_t *cr_;
	const info_t& tile_;

	std::vector<point<double>> grid_outline_;

	bool colour_by_orientation_;
};

template<typename grid>
edgeset<typename grid::coord_t> Visualizer<grid>::getUniqueTileEdges() const
{
	edgeset_t ret {};

	for (const point_t& pt : tile_.getShape() ) {
		std::vector<point_t> verts = grid::getCellVertices( pt );
		point_t last = verts.back();
		for( const point_t& cur : verts ) {
			// edges.emplace_back( last, cur );
			edge_t e { last, cur };
			edge_t opp { cur, last };
			if( ret.find( opp ) == ret.end() ) {
				ret.insert( e );
			} else {
				ret.erase( opp );
			}
			last = cur;
        }
    }

	// Automatic temp move semantics, right?
	return ret;
}

template<typename grid>
void Visualizer<grid>::initGridOutline() 
{
	if( tile_.getRecordType() == info_t::HOLE ) {
		// Don't try this if the shape has a hole.
		return;
	}

	edgeset_t eset = getUniqueTileEdges();
	point_map<coord_t,point_t> mp {};

	for( const auto& e : eset ) {
		mp[e.first] = e.second;
	}

/*
	for( const auto& a : mp ) {
		std::cerr << "   " << a.first << " -> " << a.second << std::endl;
	}
*/

	point_t start = mp.begin()->first;
	point_t v = start;

	do {
		grid_outline_.push_back( grid::vertexToGrid( v ) );
		v = mp[v];
	} while (v != start);
}

template<typename grid>
void Visualizer<grid>::drawInconclusive() const
{
	if( tile_.numPatches() > 0 ) {
		drawPatch( tile_.getPatch( 0 ), true );
	} else {
		drawShape( false );
	}
}

template<typename grid>
void Visualizer<grid>::drawNontiler( bool just_hc ) const
{
	size_t hc = tile_.getHeeschConnected();
	size_t hh = tile_.getHeeschHoles();

	if( tile_.numPatches() == 0 ) {
		drawShape( false );
		return;
	}

	if( (hc != hh) && !just_hc ) {
		cairo_save( cr_ );
		cairo_translate( cr_, 125.0, 0.0 );
		cairo_scale( cr_, 0.5, 0.5 );
		drawPatch( tile_.getPatch( 0 ), colour_by_orientation_ );
		cairo_restore( cr_ );

		cairo_save( cr_ );
		cairo_translate( cr_, 125.0, 250.0 );
		cairo_scale( cr_, 0.5, 0.5 );
		drawPatch( tile_.getPatch( 1 ), colour_by_orientation_ );
		cairo_restore( cr_ );
	} else {
		drawPatch( tile_.getPatch( 0 ), colour_by_orientation_ );
	}
}	

// https://stackoverflow.com/questions/3018313/algorithm-to-convert-rgb-to-hsv-and-hsv-to-rgb-in-range-0-255-for-both
inline colour hsv2rgb( colour rgb )
{
    double      hh, p, q, t, ff;
    long        i;

    if( rgb[1] <= 0.0 ) {       // < is bogus, just shuts up warnings
		return colour { rgb[2], rgb[2], rgb[2] };
	}
    hh = rgb[0];
    if( hh >= 360.0 ) {
		hh = 0.0;
	}
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = rgb[2] * (1.0 - rgb[1]);
    q = rgb[2] * (1.0 - (rgb[1] * ff));
    t = rgb[2] * (1.0 - (rgb[1] * (1.0 - ff)));

    switch( i ) {
		case 0: return colour { rgb[2], t, p };
		case 1: return colour { q, rgb[2], p };
		case 2: return colour { p, rgb[2], t };
		case 3: return colour { p, q, rgb[2] };
		case 4: return colour { t, p, rgb[2] };
		default: return colour { rgb[2], p, q };
	}
}

template<typename coord>
void encodeTransform( const xform<coord>& T, size_t& code, bool& ref )
{
	code = 27*(T.a_+1) + 9*(T.b_+1) + 3*(T.d_+1) + (T.e_+1);
	ref = ((T.a_*T.e_)-(T.b_*T.d_)) < 0;
}

template<typename Collection>
void computeBounds( const Collection& coll, 
	double& xmin, double& xmax, double& ymin, double& ymax )
{
	bool start = true;

	for( const auto& el : coll ) {
		double sxmin;
		double sxmax;
		double symin;
		double symax;
		computeBounds( el, sxmin, sxmax, symin, symax );

		if( start ) {
			xmin = sxmin;
			xmax = sxmax;
			ymin = symin;
			ymax = symax;
			start = false;
		} else {
			xmin = std::min( xmin, sxmin );
			xmax = std::max( xmax, sxmax ); 
			ymin = std::min( ymin, symin );
			ymax = std::max( ymax, symax );
		}
	}
}

template<typename coord>
void computeBounds( const point<coord>& pt,
	double& xmin, double& xmax, double& ymin, double& ymax )
{
	xmin = pt.x_;
	xmax = pt.x_;
	ymin = pt.y_;
	ymax = pt.y_;
}

template<typename grid>
void Visualizer<grid>::drawPatch( const patch_t& patch, bool cbyo ) const
{
	std::vector<std::vector<point<double>>> outlines {};

	for( const auto& p : patch ) {
		xform<double> Td { p.second };
		std::vector<point<double>> pts;
		for( const auto& pt : grid_outline_ ) {
			point<double> tpt = grid::gridToPage( Td * pt );
			pts.push_back( tpt );
		}
		outlines.push_back( std::move( pts ) );
	}

	double xmin = 0.0;
	double xmax = 0.0;
	double ymin = 0.0;
	double ymax = 0.0;
	computeBounds( outlines, xmin, xmax, ymin, ymax );

	cairo_save( cr_ );
	double lw = centreToBounds( xmin, xmax, ymin, ymax );
	cairo_set_line_width( cr_, lw );

	size_t code;
	bool ref;
	std::unordered_map<int,colour> ori_cols;

	if( cbyo ) {
		size_t didx = 0;
		size_t ridx = 0;

		for( size_t idx = 0; idx < grid::num_orientations; ++idx ) {
			encodeTransform( grid::orientations[idx], code, ref );
			if( ref ) {
				ori_cols.emplace( code, hsv2rgb( 
					colour { ridx * 60.0, 0.75, 0.9 } ) );
				++ridx;
			} else {
				ori_cols.emplace( code, hsv2rgb( 
					colour { didx * 60.0, 0.75, 0.4 } ) );
				++didx;
			}
		}
	}
	
	for( size_t idx = 0; idx < outlines.size(); ++idx ) {
		const std::vector<point<double>>& pts = outlines[idx];

		if( cbyo ) {
			encodeTransform( patch[idx].second, code, ref );
			const auto& col = ori_cols[code];

			drawPolygon( pts, col[0], col[1], col[2] );
		} else {
			static const double kernel[] = { 1.0, 1.0, 0.4 };
			static const double evens[] = { 0.5, 0.5, 0.5 };
			static const double odds[] = { 0.8, 0.8, 0.8 };

			if( patch[idx].first == 0 ) {
				drawPolygon( pts, kernel[0], kernel[1], kernel[2] );
			} else if( (patch[idx].first % 2) == 0 ) {
				drawPolygon( pts, evens[0], evens[1], evens[2] );
			} else {
				drawPolygon( pts, odds[0], odds[1], odds[2] );
			}
		}
	}

	cairo_restore( cr_ );
}

template<typename grid>
void Visualizer<grid>::drawShapeCells( bool dashes ) const
{
	// A more lenient routine that can handle any old crappy shape, even
	// one with holes.
	edgeset_t boundary = getUniqueTileEdges();
	std::vector<std::vector<point<double>>> outlines {};
	std::vector<point<double>> ol {};

	for( const auto& p: tile_.getShape() ) {
		ol.clear();
		std::vector<point_t> verts = grid::getCellVertices( p );
		for( const point_t& v : verts ) {
			ol.push_back( grid::gridToPage( grid::vertexToGrid( v ) ) );
        }
		outlines.push_back( std::move( ol ) );
	}

	double xmin = 0.0;
	double xmax = 0.0;
	double ymin = 0.0;
	double ymax = 0.0;
	computeBounds( outlines, xmin, xmax, ymin, ymax );

	cairo_save( cr_ );
	double lw = centreToBounds( xmin, xmax, ymin, ymax );
	cairo_set_line_width( cr_, 0.5 * lw );
	double dashpat[] = { 4.0 * lw, 4.0 * lw };
	cairo_set_dash( cr_, dashpat, 2, 0.0 );

	for( size_t idx = 0; idx < outlines.size(); ++idx ) {
		drawPolygon( outlines[idx], 1.0, 1.0, 0.4, dashes );
	}

	cairo_set_line_width( cr_, lw );
	cairo_set_line_cap( cr_, CAIRO_LINE_CAP_ROUND );
	cairo_set_dash( cr_, nullptr, 0, 0.0 );
	cairo_set_source_rgb( cr_, 0.0, 0.0, 0.0 );
	for( const auto& e : boundary ) {
		point<double> P = grid::gridToPage( grid::vertexToGrid( e.first ) );
		point<double> Q = grid::gridToPage( grid::vertexToGrid( e.second ) );
		cairo_new_path( cr_ );
		cairo_move_to( cr_, P.x_, P.y_ );
		cairo_line_to( cr_, Q.x_, Q.y_ );
		cairo_stroke( cr_ );
	}

	cairo_restore( cr_ );
}

template<typename grid>
void Visualizer<grid>::drawShape( bool dashes ) const
{
	if( dashes || (tile_.getRecordType() == info_t::HOLE) ) {
		drawShapeCells( dashes );
	} else {
		patch_t one {};
		one.emplace_back( 0, xform_t {} );
		drawPatch( one );
	}
}

template<typename grid>
void Visualizer<grid>::drawPolygon( const std::vector<point<double>>& pts,
	double r, double g, double b, bool stroke ) const
{
	cairo_new_path( cr_ );
	cairo_move_to( cr_, pts[0].x_, pts[0].y_ );
	for( size_t vidx = 1; vidx < pts.size(); ++vidx ) {
		cairo_line_to( cr_, pts[vidx].x_, pts[vidx].y_ );
	}

	cairo_close_path( cr_ );	
	cairo_set_source_rgb( cr_, r, g, b );

	if( stroke ) {
		cairo_fill_preserve( cr_ );
		cairo_set_source_rgb( cr_, 0.0, 0.0, 0.0 );
		cairo_stroke( cr_ );
	} else {
		cairo_fill( cr_ );
	}
}

template<typename grid>
void Visualizer<grid>::drawText( double x, double y ) const
{
	static const std::pair<GridType, const char *> grid_names[] = {
		{ OMINO, "Polyomino" },
		{ HEX, "Polyhex" },
		{ IAMOND, "Polyiamond" },
		{ OCTASQUARE, "Poly-[4.8.8]" },
		{ TRIHEX, "Poly-[3.6.3.6]" },
		{ ABOLO, "Polyabolo" },
		{ DRAFTER, "Polydrafter" },
		{ KITE, "Polykite" },
		{ HALFCAIRO, "Polyhalfcairo" },
		{ NOGRID, "Unknown polyform" }
	};

	std::ostringstream out {};

	cairo_set_font_size( cr_, 16 );
	cairo_font_extents_t extents;
	cairo_font_extents( cr_, &extents );
	double h = extents.height;

	for( const auto& p : grid_names ) {
		if( p.first == grid::grid_type ) {
			out << p.second;
			break;
		}
	}

	drawTextLine( out, x, y, h );

	size_t seen = 0;
	for( const auto& p : tile_.getShape() ) {
		out << p.x_ << ' ' << p.y_ << ' ';
		++seen;
		if( seen == 15 ) {
			seen = 0;
			drawTextLine( out, x, y, h );
		}
	}

	if( seen != 0 ) {
		drawTextLine( out, x, y, h );
	}

	switch( tile_.getRecordType() ) {
		case info_t::HOLE:
			out << "Has hole";
			break;
		case info_t::INCONCLUSIVE:
			out << "Analysis inconclusive";
			break;
		case info_t::NONTILER:
			out << "Nontiler, Hc = " << tile_.getHeeschConnected() 
				<< ", Hh = " << tile_.getHeeschHoles();
			break;
		case info_t::ISOHEDRAL:
			out << "Isohedral";
			break;
		case info_t::ANISOHEDRAL:
			out << "Anisohedral, " << tile_.getTransitivity()
				<< " transitivity classes";
			break;
		case info_t::APERIODIC:
			out << "Aperiodic";
			break;
		case info_t::UNKNOWN: default:
			out << "Unprocessed shape";
			break;
	}

	drawTextLine( out, x, y, h );
}
