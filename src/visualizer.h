#pragma once

#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>

#include <boost/functional/hash.hpp>

#include <cairo.h>

#include "geom.h"
#include "polyform.h"
#include "heesch.h"
#include "tileio.h"

template<typename point>
using edge = std::pair<point,point>;

template<typename grid>
class Visualizer
{
	using coord_t = typename grid::coord_t;
	using point_t = typename grid::point_t;
	using xform_t = typename grid::xform_t;
	using patch_t = Solution<coord_t>;
	using edge_t = edge<point_t>;

public:
	Visualizer( cairo_t *cr, const TileInfo<grid>& tile )
		: cr_ { cr }
		, tile_ { tile }
		, colour_by_orientation_ { false }
	{
		initGridOutline();
	}

	void setColourByOrientation( bool cbo )
	{
		colour_by_orientation_ = cbo;
	}

	void drawPatch( bool just_hc = false ) const;
	void drawShape() const;

private:
	void drawPatch( const patch_t& patch ) const;
	void drawPolygon( const std::vector<point<double>>& pts,
		double r, double g, double b ) const;

	void getTileEdges( std::vector<edge_t>& edges ) const;
	void initGridOutline();

	cairo_t *cr_;
	const TileInfo<grid>& tile_;

	std::vector<point<double>> grid_outline_;

	bool colour_by_orientation_;
};

template<typename grid>
void Visualizer<grid>::getTileEdges( std::vector<edge_t>& edges ) const
{
	for (const point_t& pt : tile_.getShape() ) {
		std::vector<point_t> verts = grid::getCellVertices( pt );
		point_t last = verts.back();
		for( const point_t& cur : verts ) {
			edges.emplace_back( last, cur );
			last = cur;
        }
    }
}

template<typename grid>
void Visualizer<grid>::initGridOutline() 
{
	// FIXME -- this won't work for shapes with holes, of course...

	std::vector<edge_t> edges;
	getTileEdges( edges );

	std::unordered_set<edge_t, boost::hash<edge_t>> eset {};

	for( const auto& e : edges ) {
		edge_t opp { e.second, e.first };
		if( eset.find( opp ) == eset.end() ) {
			eset.insert( e );
		} else {
			eset.erase( opp );
		}
	}
	
	point_map<coord_t,point_t> mp {};

	for( const auto& e : eset ) {
		mp[e.first] = e.second;
	}

	point_t start = mp.begin()->first;
	point_t v = start;

	do {
		grid_outline_.push_back( grid::vertexToGrid( v ) );
		v = mp[v];
	} while (v != start);
}

template<typename grid>
void Visualizer<grid>::drawPatch( bool just_hc ) const
{
	size_t hc = tile_.getHeeschConnected();
	size_t hh = tile_.getHeeschHoles();

	cairo_move_to( cr_, 72, 60 );
	cairo_show_text( cr_, "hello" );

	if( (hc != hh) && !just_hc ) {
		cairo_save( cr_ );
		cairo_translate( cr_, 125.0, 0.0 );
		cairo_scale( cr_, 0.5, 0.5 );
		drawPatch( tile_.getHeeschConnectedPatch() );
		cairo_restore( cr_ );

		cairo_save( cr_ );
		cairo_translate( cr_, 125.0, 250.0 );
		cairo_scale( cr_, 0.5, 0.5 );
		drawPatch( tile_.getHeeschHolesPatch() );
		cairo_restore( cr_ );
	} else {
		// Just Hc
		drawPatch( tile_.getHeeschConnectedPatch() );
	}
}	

using colour = std::array<double,3>;

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

template<typename grid>
void Visualizer<grid>::drawPatch( const patch_t& patch ) const
{
	std::vector<std::vector<point<double>>> outlines {};
	bool start = true;
	double xmin = 0.0;
	double xmax = 0.0;
	double ymin = 0.0;
	double ymax = 0.0;

	for( const auto& p : patch ) {
		xform<double> Td { p.second };
		std::vector<point<double>> pts;
		for( const auto& pt : grid_outline_ ) {
			point<double> tpt = grid::gridToPage( Td * pt );
			pts.push_back( tpt );

			if( start ) {
				xmin = tpt.x_;
				xmax = tpt.x_;
				ymin = tpt.y_;
				ymax = tpt.y_;
				start = false;
			} else {
				xmin = std::min( xmin, tpt.x_ );
				xmax = std::max( xmax, tpt.x_ ); 
				ymin = std::min( ymin, tpt.y_ );
				ymax = std::max( ymax, tpt.y_ );
			}
		}
		outlines.push_back( std::move( pts ) );
	}

	double sc = std::min( 450.0 / (xmax-xmin), 450.0 / (ymax-ymin) );
	cairo_save( cr_ );
	cairo_translate( cr_, 250.0, 250.0 );
	cairo_scale( cr_, sc, sc );
	cairo_translate( cr_, -0.5*(xmin+xmax), -0.5*(ymin+ymax) );
	
	double lwx = 1.0;
	double lwy = 0.0;
	cairo_user_to_device_distance( cr_, &lwx, &lwy );
	cairo_set_line_width( cr_, 1.0 / sqrt( lwx*lwx + lwy*lwy ) );

	size_t code;
	bool ref;
	std::unordered_map<int,colour> ori_cols;

	if( colour_by_orientation_ ) {
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

		if( colour_by_orientation_ ) {
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
void Visualizer<grid>::drawShape() const
{
	patch_t one {};
	one.emplace_back( 0, xform_t {} );
	drawPatch( one );
}

template<typename grid>
void Visualizer<grid>::drawPolygon( const std::vector<point<double>>& pts,
	double r, double g, double b ) const
{
	cairo_new_path( cr_ );
	cairo_move_to( cr_, pts[0].x_, pts[0].y_ );
	for( size_t vidx = 1; vidx < pts.size(); ++vidx ) {
		cairo_line_to( cr_, pts[vidx].x_, pts[vidx].y_ );
	}

	cairo_close_path( cr_ );	
	cairo_set_source_rgb( cr_, r, g, b );
	cairo_fill_preserve( cr_ );
	cairo_set_source_rgb( cr_, 0.0, 0.0, 0.0 );
	cairo_stroke( cr_ );
}

template<typename grid>
class VisualizerOld {
    using point_t = typename grid::point_t;
    using coord_t = typename grid::coord_t;
    using xform_t = typename grid::xform_t;
    using xformList_t = typename std::vector<std::pair<int, xform_t>>;
    using outlineList_t = typename std::vector<std::vector<point<double>>>;

    struct Tiling {
        Polyform<grid> poly;
        int hc, hh;
        xformList_t xforms;
    };

    const std::string FONT = "Georgia";

    std::istream &in;
    std::ostream &out;
    const bool holesAllowed;

    std::vector<Tiling> tilings;
    std::vector<int> hcs, hhs;
    int maxH;
    int maxLevel;
	bool maxOnly;

	bool colour_by_orientation;
	std::unordered_map<int,double> ori_cols;

public:
    VisualizerOld(std::istream &in, std::ostream &out, bool holesAllowed, int maxLevel, bool only_max, bool ori_col):
        in{in}, out{out}, holesAllowed{holesAllowed},
        tilings{}, hcs{}, hhs{}, maxH{-1}, 
		maxLevel{maxLevel}, maxOnly{only_max},
		colour_by_orientation{ori_col}, ori_cols{}
	{};

    void run() {
        init();
        printPSHeader();
        printHeeschInfo();
        // Print only the tiling(s) with maximum Heesch number
        for (auto &t : tilings) {
			if( maxOnly ) {
				// Only visualize max
				if (holesAllowed ? t.hh == maxH : t.hc == maxH) printPS(t);
			} else {
				// Visualize everything
				printPS( t );
			}
        }
        printPSFooter();
    }

private:
    void init() {
        readTilings();
        getTotalHcs(), getTotalHhs();
        // Calculate max Heesch number to be displayed
        for (int i = 0; i < std::min(maxLevel, (int) hhs.size()); ++i) {
            if ((holesAllowed ? hhs[i] : hcs[i]) > 0) maxH = i;
        }
    }

    void printPSHeader() {
        out << "%!PS-Adobe-3.0\n\n";
        out << "1 setlinewidth 0 setgray\n";
        out << "/" << FONT << " findfont\n";
        out << "12 scalefont\n";
        out << "setfont\nnewpath\n";
    }

    void printPSFooter() { out << "\n\n%%EOF\n"; }

    void printHeeschInfo() {
        std::string text = "# of polyforms per Hc: [ ";
        for (int n : hcs) text += std::to_string(n) + " ";
        text += "]\n";
        printText(72, 720, text);

        text = "# of polyforms per Hh: [ ";
        for (int n : hhs) text += std::to_string(n) + " ";
        text += "]";
        printText(72, 700, text);

        text = "Total # of polyforms: "; //  + std::to_string(std::accumulate(hcs.begin(), hcs.end(), 0));
        printText(72, 680, text);

        out << "showpage\n";
    }

    void printText(int x, int y, const std::string &text) {
        out << x << " " << y << " moveto\n";
        out << "(" << text << ") show\n";
    }

    void readTilings() {
		size_t holes = 0;

        tilings.clear();
        while (in.peek() != EOF) {
			auto tiling = readTiling();
			if( tiling.poly.simplyConnected() ) {
				tilings.push_back(tiling);
			} else {
				tiling.poly.debug();
				++holes;
			}
			// tilings.push_back(readTiling());
		}

		if( holes > 0 ) {
			std::cerr << "Skipped " << holes << " shapes with internal holes"
				<< std::endl;
		}
    }

    Tiling readTiling() {
        auto pts = readPoints();
        auto heeschNums = readHeeschNums();
        auto hc = heeschNums.first, hh = heeschNums.second;
        xformList_t xformsHc = readXforms();
        xformList_t xformsHh{};
        if (hh > 0 && hh != hc) xformsHh = readXforms();
        if (holesAllowed) return {Polyform<grid>{pts}, hc, hh, xformsHh};
        else return {Polyform<grid>{pts}, hc, hh, xformsHc};
    }

    std::vector<point_t> readPoints() {
        std::vector<point_t> pts{};
        coord_t x, y;
        std::stringstream line = getLine();
        while (line >> x, line >> y) pts.emplace_back(x, y);
        return pts;
    }

    std::pair<int, int> readHeeschNums() {
        auto line = getLine();
        int hc, hh;
        line >> hc >> hh;
        return {hc, hh};
    }

    xformList_t readXforms() {
        xformList_t xforms{};
        size_t nTiles;
        getLine() >> nTiles;
        for (size_t i = 0; i < nTiles; ++i) {
            auto line = getLine();
            int level;
            coord_t a, b, c, d, e, f;
            line >> level >> a >> b >> c >> d >> e >> f;
            xforms.push_back({level, xform_t{a, b, c, d, e, f}});
        }
        return xforms;
    }

    void getTotalHcs() {
        hcs.clear();
        for (auto &t : tilings) {
            if ((int) hcs.size() <= t.hc) hcs.resize(t.hc + 1);
            ++hcs[t.hc];
        }
    }

    void getTotalHhs() {
        hhs.clear();
        for (auto &t : tilings) {
            if ((int) hhs.size() <= t.hh) hhs.resize(t.hh + 1);
            ++hhs[t.hh];
        }
    }

    void printPS(const Tiling &tiling) {
        printTilingInfo(tiling);
        auto outline = getOutlineVertices(tiling);
        scaleToFitPage(outline);
        printTiles(tiling.xforms, outline);
        out << "showpage\n";
    }

    void printTilingInfo(const Tiling &tiling) {
        printText(72, 720,
                  "Polyform coords: " + tiling.poly.toString() + "\n" +
                  "Hc: " + std::to_string(tiling.hc) + "\n" +
                  "Hh: " + std::to_string(tiling.hh));
    }

    outlineList_t getOutlineVertices(const Tiling &tiling) {
        auto outlineVertices = tiling.poly.getGridOutlineVertices();
        std::vector<std::vector<point<double>>> answer(tiling.xforms.size());
        for (size_t i = 0; i < answer.size(); ++i) {
            xform<double> x{tiling.xforms[i].second};
            for (auto &pt : outlineVertices)
                answer[i].push_back(grid::gridToPage(x * pt));
        }
        return answer;
    }

    void scaleToFitPage(outlineList_t &outline) {
        auto bs = getBounds(outline);
        double pcX = 4.25 * 72, pcY = 5.5 * 72;
        double cX = 0.5 * (bs.xMin + bs.xMax), cY = 0.5 * (bs.yMin + bs.yMax);
        double horizScale = (6.5 * 72) / (bs.xMax - bs.xMin + 2);
        double vertScale = (9.0 * 72) / (bs.yMax - bs.yMin + 2);
        double scale = std::min(horizScale, vertScale);

        for (auto &tile : outline) {
            for (auto &pt : tile) {
                pt.x_ = pcX + scale * (pt.x_ - cX);
                pt.y_ = pcY + scale * (pt.y_ - cY);
            }
        }
    }

    struct Bounds {
        double xMin, yMin, xMax, yMax;
    };

    Bounds getBounds(const outlineList_t &outline) {
        double xMin = 1e9, yMin = 1e9, xMax = -1e9, yMax = -1e9;
        for (auto &tile : outline) {
            for (auto &pt : tile) {
                xMin = std::min(xMin, pt.x_);
                yMin = std::min(yMin, pt.y_);
                xMax = std::max(xMax, pt.x_);
                yMax = std::max(yMax, pt.y_);
            }
        }
        return {xMin, yMin, xMax, yMax};
    }

    void printTiles(const xformList_t &xforms, const outlineList_t &outline) {
        for (size_t i = 0; i < xforms.size(); ++i) {
            int level = xforms[i].first;
			xform T = xforms[i].second;
            bool start = true;
            for (auto &pt : outline[i]) {
                out << pt.x_ << ' ' << pt.y_ << ' ';
                if (start) start = false, out << "moveto";
                else out << "lineto";
                out << '\n';
            }
			const auto& pt = outline[i][0];
			out << pt.x_ << ' ' << pt.y_ << " lineto" << std::endl;

            out << "closepath\n";
			double colour = 0.2;
			bool flipped = false;

			if( colour_by_orientation ) {
				int code = 27*(T.a_+1) + 9*(T.b_+1) + 3*(T.d_+1) + (T.e_+1);
				int det = (T.a_*T.e_)-(T.b_*T.d_);

				if( ori_cols.find( code ) == ori_cols.end() ) {
					colour = 0.4 + 0.4 * double(ori_cols.size()) / 
						double(grid::num_orientations);
					ori_cols[code] = colour;
				} else {
					colour = ori_cols[code];
				}

				if( det < 0 ) {
					flipped = true;
				}
			} else {
				if( level > 0 ) {
					colour = (level%2==0) ? 0.5 : 0.8;
				}
			}
				
			if( level == 0 ) {
				out << "gsave 1 1 0 setrgbcolor fill grestore stroke newpath\n";
			} if( flipped ) {
				out << "gsave 0.9 0.3 " << colour << " setrgbcolor fill grestore stroke newpath\n";
			} else {
				out << "gsave " << colour << " setgray fill grestore stroke newpath\n";
			}
        }
    }

    std::stringstream getLine() {
        std::string str;
        getline(in, str);
        // Replace all non-number characters with spaces
        for (char &c : str) if (!isdigit(c) && c != '-') c = ' ';
        return std::stringstream{str};
    }
};
