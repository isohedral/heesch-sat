#pragma once

#include "shape.h"

template <typename vector>
vector lexoLeastRotation(vector v) {
    auto least = v;
    for (size_t i = 1; i < v.size(); ++i) {
        std::rotate(v.begin(), v.begin() + 1, v.end());
        if (v < least) least = v;
    }
    return least;
}

template <typename grid>
class Polyform {
    using point_t = typename grid::point_t;
    using edge_t = std::pair<point_t, point_t>;

    std::vector<int8_t> boundaryWord;
    Shape<grid> shape;
    std::vector<point_t> pts;

public:
    explicit Polyform(std::vector<point_t> p): boundaryWord{}, shape{}, pts{std::move(p)} {
        for (auto pt : pts) shape.add(pt);
        //fillBoundaryWord();
        //canonizalizeBoundaryWord();
    }

    void printBoundary() {
        for (auto x : boundaryWord) std::cout << (int) x << ' ';
        std::cout << '\n';
    }

    bool operator<(const Polyform<grid> &other) const {
        if (boundaryWord.size() == other.boundaryWord.size()) return boundaryWord < other.boundaryWord;
        return boundaryWord.size() < other.boundaryWord.size();
    }
    bool operator==(const Polyform<grid> &other) const {
        return boundaryWord == other.boundaryWord;
    }

    [[nodiscard]] bool simplyConnected() const { return shape.simplyConnected(); }

    [[nodiscard]] std::string toString() const {
        std::string ans;
        for (const auto &pt : shape) ans += std::to_string(pt.getX()) + ' ' + std::to_string(pt.getY()) + ' ';
        return ans;
    }

    // Returns outline vertices, xformed to the grid coordinate system
    [[nodiscard]] std::vector<point<double>> getGridOutlineVertices() const {
        auto outlineVertices = getOutlineVertices();
        std::vector<point<double>> answer{};
        for (auto pt : outlineVertices) answer.push_back(grid::vertexToGrid(pt));
        return answer;
    }

private:

    std::vector<point_t> getOutlineVertices() const {
        auto edges = getAllTileEdges();

        std::sort(edges.begin(), edges.end(), compare);
        std::map<point_t, point_t> mp{};
        for (size_t i = 0; i < edges.size(); ++i) {
            if (i != edges.size() - 1 && equals(edges[i], edges[i + 1])) ++i;
            else mp[edges[i].first] = edges[i].second;
        }

        point_t start = mp.begin()->first;
        point_t v = start;
        std::vector<point_t> vertices{};
        do {
            vertices.push_back(v);
            v = mp[v];
        } while (v != start);
        return vertices;
    }

    void fillBoundaryWord() {
        auto outlineVertices = getOutlineVertices();
        for (size_t i = 0; i < outlineVertices.size(); ++i) {
            auto dir = outlineVertices[(i+1) % outlineVertices.size()] - outlineVertices[i];
            boundaryWord.push_back(grid::getBoundaryWordDirection(dir));
        }
    }

    void canonizalizeBoundaryWord() {
        std::vector<int8_t> word = canonicalRotation(boundaryWord);
        std::vector<int8_t> reflectedWord = canonicalRotation(reflectBoundaryWord(boundaryWord));
        boundaryWord = word < reflectedWord ? word : reflectedWord;
    }

    std::vector<int8_t> canonicalRotation(std::vector<int8_t> word) {
        std::vector<int8_t> least = lexoLeastRotation(word);
        for (size_t i = 0; i < grid::numRotations() - 1; ++i) {
            word = lexoLeastRotation(rotateBoundaryWord(word));
            if (word < least) least = word;
        }
        return least;
    }

    std::vector<int8_t> rotateBoundaryWord(std::vector<int8_t> word) {
        for (auto &dir : word) dir = grid::rotateDirection(dir);
        return word;
    }

    std::vector<int8_t> reflectBoundaryWord(std::vector<int8_t> word) {
        for (auto &dir : word) dir = grid::reflectDirection(dir);
        std::reverse(word.begin(), word.end());
        return word;
    }

    std::vector<edge_t> getAllTileEdges() const {
        std::vector<edge_t> edges;
        for (const point_t &pt : pts) {
            auto cellEdges = grid::getCellEdges(pt);
            edges.insert(edges.end(), cellEdges.begin(), cellEdges.end());
        }
        return edges;
    }

    static void canon(edge_t &x) { if (x.second < x.first) std::swap(x.first, x.second); }

    static bool compare(edge_t a, edge_t b) {
        canon(a), canon(b);
        return a.first == b.first ? a.second < b.second : a.first < b.first;
    }

    static bool equals(edge_t a, edge_t b) {
        canon(a), canon(b);
        return a.first == b.first && a.second == b.second;
    }
};

template <typename grid>
inline std::ostream& operator <<(std::ostream &os, const Polyform<grid> &poly) {
    return os << poly.toString();
}
