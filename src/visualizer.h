#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <cctype>
#include <numeric>
#include <unordered_map>

#include "geom.h"
#include "polyform.h"

template<typename grid>
class Visualizer {
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
    Visualizer(std::istream &in, std::ostream &out, bool holesAllowed, int maxLevel, bool only_max, bool ori_col):
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

        text = "Total # of polyforms: " + std::to_string(std::accumulate(hcs.begin(), hcs.end(), 0));
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
				++holes;
			}
			// tilings.push_back(readTiling());
		}

		std::cerr << "Skipped " << holes << " shapes with internal holes"
			<< std::endl;
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
