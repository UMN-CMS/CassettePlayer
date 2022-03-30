#include <structopt/app.hpp>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "core/slots.h"
#include "vmath/vmath.h"
#include "rapidcsv.h"
#include "spdlog/spdlog.h"

struct Row {
    int u, v, plane;
    int irot, mrot;
    std::string itype;
    PointVec vertices;
    float x0, y0;
    float phi;
    int icas_0, icas_1;
    bool is_engine;
    int MB;
};

std::vector<Row> extractRows(const std::string& file);

struct Channel {
    const std::vector<Row>& rows;
    std::vector<SlotCassette> cass;
    std::vector<SlotModule> mods;
    std::vector<SlotEngine> eng;
};

void processCassettes(std::unordered_map<CasSlot, PositionInfo>& ret,
                      Channel& c);

void processModules(std::unordered_map<CasSlot, PositionInfo>& ret, Channel& c);

void processEngines(std::unordered_map<CasSlot, PositionInfo>& ret, Channel& c);
/*
void processWagons(std::unordered_map<CasSlot, PositionInfo>& ret,
                   const std::vector<Row>& rows,
                   const std::vector<SlotModule>& found_mods) {
    for (const Row& row : rows) {

    }
}
*/

std::unordered_map<CasSlot, PositionInfo> parsePos(
    const std::vector<Row>& rows);

template <typename Functor, typename Arr, std::size_t... I>
bool applyFunctorToArray(Functor f, Arr&& ar, std::index_sequence<I...>) {
    return f(ar[I]...);
}

template <typename Vec, typename Functor, std::size_t N = 3>
int getConsecutiveIf(Vec v, Functor&& f) {
    std::size_t size = std::size(v);
    for (std::size_t i = 0; i < N; ++i) {
        v.push_back(v[i]);
    }
    for (std::size_t i = 0; i < size; ++i) {
        std::array<typename Vec::value_type, N> searches;
        for (std::size_t j = 0; j < N; ++j) {
            searches[j] = v[i + j];
            if (applyFunctorToArray(f, searches,
                                    std::make_index_sequence<N>{})) {
                return i;
            };
        }
    }
    return -1;
}

template <typename Vec>
int getThreeWithAngle(const Vec& v) {
    return getConsecutiveIf(v, [](auto x, auto y, auto z) {
        return std::abs(std::abs(threePointAngle(x, y, z)) - (4 * 3.1415) / 6) <
               0.01;
    });
}

template <typename Vec>
int getThreeWithAngleLess(const Vec& v, int l) {
    return getConsecutiveIf(v, [l](auto x, auto y, auto z) {
        return (std::abs(std::abs(threePointAngle(x, y, z)) -
                         (4 * 3.1415) / 6) < 0.01) &&
               (norm(x - y) < l && norm(y - z) < l);
    });
}

template<typename T>
T orthDirection(const T& p) {
    Point p2 = {-p.second, p.first};
    p2 = p2 / norm(p2);
    return p2;
}

struct Ray {
    Point start, dir;
};
