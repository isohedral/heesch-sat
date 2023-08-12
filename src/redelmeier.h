#pragma once

#include <set>
#include <vector>

#include "grid.h"

template<typename grid>
using polyform_cb = 
	std::function<void( const std::vector<typename grid::point_t>& )>;

// Just enumerate fixed polyforms over this grid.
template<typename grid, typename CB = polyform_cb<grid>>
class RedelmeierSimple
{
    using coord_t = typename grid::coord_t;
    using point_t = typename grid::point_t;

    enum CellStatus {
        FREE, OCCUPIED, UNTRIED, BLOCKED, REACHABLE
    };

	using cell_map = point_map<coord_t, CellStatus>;

	CB out;
	size_t size;

public:
	explicit RedelmeierSimple( CB out, size_t sz )
		: out { out }
		, size { sz }
	{}

	size_t solve()
	{
		cell_map shape;
		size_t total = 0;
		for( const auto& o : grid::origins ) {
			std::vector<point_t> untried = { o };
			total += solve( 0, o, shape, untried, 0 );
		}

		return total;
	}

private:
	bool contains( cell_map& shape, const point_t& p )
	{
		return shape.find( p ) != shape.end();
	}

	size_t solve( size_t sz, const point_t& origin, cell_map& shape, 
		std::vector<point_t>& untried, size_t from );
};

template<typename grid, typename CB>
size_t RedelmeierSimple<grid, CB>::solve(
	size_t sz, const RedelmeierSimple::point_t& origin, 
	RedelmeierSimple::cell_map& shape,
	std::vector<RedelmeierSimple::point_t>& untried, size_t from )
{
	//	std::cerr << "Solving " << sz << std::endl;

	if( sz == size ) {
		std::vector<point_t> pts;

		for( auto& p : shape ) {
			if( p.second == OCCUPIED ) {
				pts.push_back( p.first );
			}
		}

		out( pts );
		return 1;
	} else {
		size_t total = 0;
		size_t usz = untried.size();

		for( size_t idx = from; idx < usz; ++idx ) {
			point_t p = untried[idx];
			shape[p] = OCCUPIED;

			for( auto pn : edge_neighbours<grid> { p } ) {
				if( !(pn < origin) && !contains( shape, pn ) ) {
					// FIXME -- this is an inefficient way to check
					// whether this is a new untried cell.
					if( std::find( untried.begin(), untried.end(), pn )
							== untried.end() ) {
						untried.push_back( pn );
					}
				}
			}

			total += solve( sz + 1, origin, shape, untried, idx + 1 );
			shape[p] = REACHABLE;
			untried.resize( usz );
		}

		for( size_t idx = from; idx < usz; ++idx ) {
			shape.erase( untried[idx] );
		}

		return total;
	}
}

#if 0

template<typename grid>
class Redelmeier {
    using point_t = typename grid::point_t;
    using TileType = typename grid::TileType;

    enum CellStatus {
        FREE, OCCUPIED, UNTRIED, BLOCKED, REACHABLE
    };

    std::ostream &out;
    point_t origin;
    std::vector<int8_t> tilesLeft;
    std::map<point_t, CellStatus> cells;
    std::vector<point_t> occupiedCells;
    std::vector<Polyform<grid>> symmetricPolys;
    size_t totalPolys;

public:
    explicit Redelmeier(std::ostream &out, std::vector<int8_t> numTiles):
        out{out}, origin{}, tilesLeft{std::move(numTiles)},
        cells{}, occupiedCells{}, symmetricPolys{}, totalPolys{0}
    {}

    size_t solve() {
        symmetricPolys.clear();
        totalPolys = 0;
        for (size_t t = 0; t < grid::numTileTypes(); ++t) {
            if (tilesLeft[grid::getTileShape((TileType) t)] > 0) {
                cells.clear();
                occupiedCells.clear();
                origin = grid::getTileTypeOrigin((TileType) t);
                cells[origin] = UNTRIED;
                solveHelper({origin});
            }
        }
        addSymmetricPolysToAnswer();
        return totalPolys;
    }

private:
    void addSymmetricPolysToAnswer() {
        std::sort(symmetricPolys.begin(), symmetricPolys.end());
        symmetricPolys.erase(std::unique(symmetricPolys.begin(), symmetricPolys.end()), symmetricPolys.end());
        for (auto &poly : symmetricPolys) printPoly(poly);
    }

    void printPoly(const Polyform<grid> &poly) {
        out << poly << '\n';
        ++totalPolys;
    }

    void solveHelper(const std::vector<point_t> &untriedList) {
        if (noTilesAreLeft()) {
            addPolyToAnswer(Polyform<grid>{occupiedCells});
            return;
        }
        for (size_t i = 0; i < untriedList.size(); ++i)
            if (canPlaceTile(untriedList[i]))
                placeTile(i, untriedList[i], untriedList);
        for (const auto &pos : untriedList) cells[pos] = UNTRIED;
    }

    void addPolyToAnswer(const Polyform<grid> &poly) {
        if (!poly.simplyConnected()) return;
        if (poly.isSymmetric()) symmetricPolys.push_back(poly);
        else if (poly.isCanonical()) printPoly(poly);
    }

    void placeTile(int i, const point_t &pos, const std::vector<point_t> &untriedList) {
        --tilesLeft[grid::getTileShape(pos)];
        cells[pos] = OCCUPIED;
        occupiedCells.push_back(pos);

        ++tilesLeft[grid::getTileShape(pos)];
        cells[pos] = REACHABLE;
        occupiedCells.pop_back();
    }

/*
    std::vector<point_t> getChildUntriedList(int i, const point_t &pos, const std::vector<point_t> &untriedList ) {
        std::vector<point_t> childUntriedList(untriedList.begin() + i + 1, untriedList.end());
        return childUntriedList;
    }

    std::vector<point_t> getNewUntriedList(const point_t &pos ) {
        auto newUntriedList = getNewUntriedList(pos);
        return newUntriedList;
    }

    std::vector<point_t> getNewUntriedList(const point_t &pos) {
        std::vector<point_t> newUntriedList{};
        for (const auto &nxt : edge_neighbours<grid>(pos)) {
            if (getCellStatus(nxt) == FREE) {
                cells[nxt] = UNTRIED;
                newUntriedList.emplace_back(nxt);
            }
        }
        return newUntriedList;
    }
*/

    CellStatus getCellStatus(const point_t &pos) const {
        if (pos.y_ < origin.y_ ||
            (pos.y_ == origin.y_ && pos.x_ < origin.x_))
            return BLOCKED;
        try {
            return cells.at(pos);
        } catch (std::out_of_range &e) {
            return FREE;
        }
    }

    [[nodiscard]] bool canPlaceTile(const point_t &pos) const {
        return tilesLeft[grid::getTileShape(pos)] > 0 && getCellStatus(pos) == UNTRIED;
    }

    [[nodiscard]] bool noTilesAreLeft() const {
        return std::accumulate(tilesLeft.begin(), tilesLeft.end(), 0) == 0;
    }
};

#endif
