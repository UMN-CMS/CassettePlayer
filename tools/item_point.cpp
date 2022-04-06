#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

// #include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
// #include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>
// #include <iostream>
#include <structopt/app.hpp>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "common.h"
#include "core/slots.h"
#include "rapidcsv.h"
#include "spdlog/spdlog.h"
#include "visualization/drawables.h"
#include "vmath/vmath.h"

std::unordered_map<std::string, Color> COLOR{
    {"OMODULE", 0x8000FF}, {"MMODULE", 0x6000FF}, {"IMODULE", 0x2000FF},
    {"ENGINE", 0xE000FF},  {"CON_MEZ", 0x1269FF}, {"DCDC", 0x12690F},
    {"WAGON", 0xAAAA00}, {"DEP_DCDC", 0xE000AF}
};

std::unordered_map<std::string, PointVec> wagon_shapes = {
    {"West3",
     {{65.7, 3.0},      {65.7, 13.8},     {69.08, 19.0},    {69.08, 27.0},
      {65.7, 32.2},     {65.7, 43.0},     {-4.3, 43.0},     {-101.74, 43.0},
      {-171.74, 43.0},  {-269.18, 43.0},  {-339.18, 43.0},  {-339.18, 3.0},
      {-328.78, -6.76}, {-308.78, -6.76}, {-298.38, 3.0},   {-269.18, 3.0},
      {-171.74, 3.0},   {-161.34, -6.76}, {-141.34, -6.76}, {-130.94, 3.0},
      {-101.74, 3.0},   {-4.3, 3.0},      {6.1, -6.76},     {26.1, -6.76},
      {36.5, 3.0}}},
    {"East3",
     {{53.3, 3.0},     {163.14, 3.0},  {173.54, -6.76}, {193.54, -6.76},
      {203.94, 3.0},   {220.74, 3.0},  {330.58, 3.0},   {340.98, -6.76},
      {360.98, -6.76}, {371.38, 3.0},  {388.18, 3.0},   {388.18, 43.0},
      {330.58, 43.0},  {220.74, 43.0}, {163.14, 43.0},  {53.3, 43.0},
      {-65.7, 43.0},   {-65.7, 32.2},  {-69.08, 27.0},  {-69.08, 19.0},
      {-65.7, 13.8},   {-65.7, 3.0},   {-4.3, 3.0},     {6.1, -6.76},
      {26.1, -6.76},   {36.5, 3.0}}},
    {"West2",
     {{65.7, 3.0},
      {65.7, 13.8},
      {69.08, 19.0},
      {69.08, 27.0},
      {65.7, 32.2},
      {65.7, 43.0},
      {-4.3, 43.0},
      {-114.14, 43.0},
      {-171.74, 43.0},
      {-171.74, 3.0},
      {-161.34, -6.76},
      {-141.34, -6.76},
      {-130.94, 3.0},
      {-114.14, 3.0},
      {-4.3, 3.0},
      {6.1, -6.76},
      {26.1, -6.76},
      {36.5, 3.0}}},
    {"East2",
{{220.74, 3.0}, {220.74, 43.0}, {163.14, 43.0}, {53.3, 43.0}, {-65.7, 43.0}, {-65.7, 32.2}, {-69.08, 27.0}, {-69.08, 19.0}, {-65.7, 13.8}, {-65.7, 3.0}, {-4.3, 3.0}, {6.1, -6.76}, {26.1, -6.76}, {36.5, 3.0}, {53.3, 3.0}, {163.14, 3.0}, {173.54, -6.76}, {193.54, -6.76}, {203.94, 3.0}}
    },
    {"West1",
     {{65.7, 3.0},
      {65.7, 13.8},
      {69.08, 19.0},
      {69.08, 27.0},
      {65.7, 32.2},
      {65.7, 43.0},
      {-4.3, 43.0},
      {-4.3, 3.0},
      {6.1, -6.76},
      {26.1, -6.76},
      {36.5, 3.0}}},
    {"East1",
     {{53.3, 3.0},
      {53.3, 43.0},
      {-65.7, 43.0},
      {-65.7, 32.2},
      {-69.08, 27.0},
      {-69.08, 19.0},
      {-65.7, 13.8},
      {-65.7, 3.0},
      {-4.3, 3.0},
      {6.1, -6.76},
      {26.1, -6.76},
      {36.5, 3.0}}},
    {"E_Triangle", {{53.3, 3.0},        {163.14, 3.0},      {173.54, -6.76},
                  {193.54, -6.76},    {203.94, 3.0},      {220.74, 3.0},
                  {220.74, 43.0},     {163.14, 43.0},     {53.3, 43.0},
                  {-65.7, 43.0},      {-65.7, 32.2},      {-69.08, 27.0},
                  {-69.08, 19.0},     {-65.7, 13.8},      {-65.7, 3.0},
                  {-51.384, 3.0},     {19.831, -120.348}, {48.631, -170.231},
                  {83.272, -150.231}, {86.524, -136.345}, {76.524, -119.024},
                  {62.872, -114.897}, {54.472, -100.348}, {-5.196, 3.0},
                  {-4.3, 3.0},        {6.1, -6.76},       {26.1, -6.76},
                  {36.5, 3.0}}},
    {"W_Hockey", {{65.7, 3.0},         {65.7, 13.8},        {69.08, 19.0},
                {69.08, 27.0},       {65.7, 32.2},        {65.7, 43.0},
                {-4.3, 43.0},        {-4.3, 3.0},         {19.831, -120.348},
                {48.631, -170.231},  {103.551, -265.355}, {132.351, -315.238},
                {166.992, -295.238}, {170.244, -281.352}, {160.244, -264.031},
                {146.592, -259.905}, {138.192, -245.355}, {83.272, -150.231},
                {86.524, -136.345},  {76.524, -119.024},  {62.872, -114.897},
                {54.472, -100.348},  {36.5, 3.0}}}};


const PointVec ldengine_w =
  {{41.7, 7.5}, {51.7, 7.5}, {61.7, -2.5}, {109.7, -2.5}, {119.7, 7.5}, {124.7, 7.5}, {124.7, 22.5}, {106.7, 42.5}, {106.7, 45.0}, {101.7, 55.0}, {96.7, 55.0}, {96.7, 40.0}, {66.7, 40.0}, {66.7, 56.0}, {59.7, 60.0}, {53.7, 60.0}, {53.7, 42.0}, {41.7, 25.0}};

const PointVec ldengine_e =
  {{-124.7, 7.5}, {-114.7, 7.5}, {-104.7, -2.5}, {-56.7, -2.5}, {-46.7, 7.5}, {-41.7, 7.5}, {-41.7, 22.5}, {-59.7, 42.5}, {-59.7, 45.0}, {-64.7, 55.0}, {-69.7, 55.0}, {-69.7, 40.0}, {-99.7, 40.0}, {-99.7, 56.0}, {-106.7, 60.0}, {-112.7, 60.0}, {-112.7, 42.0}, {-124.7, 25.0}};

const PointVec square = {{-15, -15}, {-15, 15}, {15, 15}, {15, -15}};
const PointVec conmez = {{-15, -15}, {-15, 15}, {15, 15}, {15, -15}};
const PointVec dcdctri = {{56.0, 45.5}, {56.0, 60.4},  {9.0, 88.0},
                          {-9.0, 88.0}, {-56.0, 60.4}, {-56.0, 45.5}};
const PointVec d_dcdctri = {{56.0, -45.5}, {56.0, -60.4},  {9.0, -88.0},
                            {-9.0, -88.0}, {-56.0, -60.4}, {-56.0, -45.5}};

template <typename T, typename U>
constexpr ElementType<U> applyTo(T&& appl, const U& arr) {
    typename std::remove_reference_t<std::remove_cv_t<U>> ret = arr;
    for (auto& thing : ret) {
        thing = appl(thing);
    }
    return ret;
}

std::optional<Point> intersectionOf(const Ray& r1, const Ray& r2) {
    auto as = r1.start;
    auto ad = r1.dir;
    auto bs = r2.start;
    auto bd = r2.dir;
    Point d = bs - as;
    float det = bd.first * ad.second - bd.second * ad.first;
    if (det == 0) return {};
    float u = (d.first * bd.first - d.first * bd.second) / det;
    // float v = (d.second * ad.first - d.second * ad.second) / det;
    return as + ad * u;
}

template <typename T>
std::pair<Point, Point> getExtrude2Point(const T& p1, const T& p2, const T& p3,
                                         float width) {
    Point bdir = bisectDir(p1, p2, p3);
    Ray biray{p2 + 2 * width * bdir, -bdir};
    Point dir1 = (p2 - p1) / norm(p2 - p1);
    Ray r1{p1 + orthDirection(p2 - p1) * width, dir1};
    Ray r2{p1 - orthDirection(p2 - p1) * width, dir1};
    auto p1_o = intersectionOf(biray, r1);
    auto p2_o = intersectionOf(biray, r2);
    return {p1_o.value(), p2_o.value()};
}

PointVec extrudeToWidth(const std::vector<Point>& points, int width) {
    if (std::size(points) < 2) return {};
    PointVec ret;
    std::size_t vlen = std::size(points);
    std::vector<std::pair<Point, Point>> accum;
    Point firstdir = (points[1] - points[0]) / norm(points[1] - points[0]);
    Point lastdir = (points[vlen - 1] - points[vlen - 2]) /
                    norm(points[vlen - 1] - points[vlen - 1]);
    accum.push_back(
        {points[0] + width * firstdir, points[0] - width * firstdir});

    for (std::size_t i = 1; i < vlen - 1; ++i) {
        accum.push_back(
            getExtrude2Point(points[i - 1], points[i], points[i + 1], width));
    }

    accum.push_back({points[vlen - 1] + width * lastdir,
                     points[vlen - 1] - width * lastdir});
    for (auto it = accum.begin(); it != accum.end(); ++it) {
        ret.push_back(it->first);
    }
    for (auto it = accum.rbegin(); it != accum.rend(); ++it) {
        ret.push_back(it->second);
    }
    return ret;
}

// constexpr float WAGONWIDTH = 20;

std::string getWagonType(const PointVec& pv) {
    constexpr float pi = 3.1415;
    constexpr float sec = pi / 6;
    if (std::size(pv) < 3) return "S";
    std::string ret;
    const char c = 'A';
    for (std::size_t i = 0; i < std::size(pv) - 2; ++i) {
        Point dir1 = pv[i + 1] - pv[i];
        Point dir2 = pv[i + 2] - pv[i + 1];
        float angle = trueAngleBetween(dir1, dir2);
        ret += (c + 3 + std::round(angle / sec));
    }
    return ret;
}

std::vector<std::vector<Row>> splitToCas(const std::vector<Row>& rows,
                                         int max) {
    std::vector<std::vector<Row>> ret;
    for (int i = 0; i < 4 * max; ++i) {
        for (std::size_t j = 0; j < 3; ++j) {
            int x = 1 & j;
            int y = 1 & (j >> 1);
            std::vector<Row> temp;
            std::copy_if(rows.begin(), rows.end(), std::back_inserter(temp),
                         [i, x, y](const auto& r) {
                             return r.plane == i && r.icas_0 == x &&
                                    r.icas_1 == y;
                         });
            ret.push_back(std::move(temp));
        }
    }
    return ret;
}

std::unordered_map<CasSlot, Drawable> parseShapes(
    const std::vector<Row>& rows) {
    std::unordered_map<CasSlot, Drawable> ret;
    std::vector<Row> uniq_rows;
    std::unordered_set<std::string> types;
    std::unordered_map<int, std::vector<Row>> wagon_roots;
    for (const Row& r : rows) {
        if (types.insert(r.itype).second) {
            uniq_rows.push_back(r);
        };
    }

    for (const Row& r : uniq_rows) {
        PointVec pv;
        int nv = std::size(r.vertices);
        int three_idx = getThreeWithAngleLess(r.vertices, 97);
        if (three_idx == -1) {
            spdlog::debug(
                "Did not find three points forming hexagonal side with "
                "type {}",
                r.itype);
            continue;
        };
        Point center = centerOfRegPolygon<Point, 6>(
            r.vertices[three_idx % nv], r.vertices[(three_idx + 1) % nv],
            r.vertices[(three_idx + 2) % nv], 96.7);

        for (Point p : r.vertices) {
            p = p - center;
            pv.push_back(std::move(p));
        }
        SlotModule sm(-1, -1, SlotCassette({-1, -1, -1}), r.itype);
        std::vector<StyledPolygon> needs_shapes;
        if (r.itype.find("O") != std::string::npos) {
            needs_shapes.push_back(StyledPolygon{pv, COLOR["OMODULE"]});
        } else if (r.itype.find("I") != std::string::npos) {
            needs_shapes.push_back(StyledPolygon{pv, COLOR["IMODULE"]});

        } else if (r.itype.find("M") != std::string::npos) {
            needs_shapes.push_back(StyledPolygon{pv, COLOR["MMODULE"]});
        }

        if (r.itype.find("F") != std::string::npos &&
            r.itype.find("I") == std::string::npos) {
            needs_shapes.push_back(
                StyledPolygon{applyTo(
                                  [](const auto& x) {
                                      return Point(x.first * 1.75 - 0 * 15,
                                                   x.second * 1 + 25);
                                  },
                                  square),
                              COLOR["CON_MEZ"]});
            needs_shapes.push_back(StyledPolygon{dcdctri, COLOR["DCDC"]});
        }
        ret.insert({sm, Drawable(needs_shapes)});
    }

    ret.insert({SlotEngine(),
                Drawable({StyledPolygon{applyTo(
                                            [](const auto& x) {
                                              return Point(x.first * 1,
                                                           x.second * -1);
                                            },
                                            ldengine_w),
                                        COLOR["ENGINE"]}})});
    ret.insert({SlotDepDCDC(),
                Drawable({StyledPolygon{applyTo(
                                            [](const auto& x) {
                                                return Point(x.first * 1,
                                                             x.second * 1);
                                            },
                                            d_dcdctri),
                                        COLOR["DEP_DCDC"]}})});
    for (const auto& pair : wagon_shapes) {
        spdlog::debug("Makin shape : {}", pair.first);
        int rot = 1;
        if (pair.first.find("East") != std::string::npos || 
            pair.first.find("West") != std::string::npos
            ) rot = 1;
        ret.insert({SlotWagon(SlotEngine(), pair.first, 0, 0),
                    Drawable({StyledPolygon(applyTo(
                                                [rot](const auto& x) {
                                                    return Point(rot * x.first,
                                                                 -x.second);
                                                },
                                                wagon_shapes[pair.first]),
                                            COLOR["WAGON"])})});
    }

    return ret;
}
/*
static const char USAGE[] =
    R"(Geoparser, a tool for extracting gometry information from the HGCAL
geometry file.

    Usage:
      geoparser [-o <outfile>] <file>

    Options:
      -o <outfile>, --output <outfile>   Output file name [default: out.xml]
      -h --help               Show this screen.
      --version               Show version.
)";
*/

struct Options {
    std::optional<std::string> outfile = "out.xml";
    enum class ParseType { points, shapes };
    std::optional<ParseType> parse = ParseType::points;
    std::string infile;
};
STRUCTOPT(Options, outfile, infile, parse);

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::debug);
    /*
      std::map<std::string, docopt::value> args =
          docopt::docopt(USAGE, {argv + 1, argv + argc},
                         true,    // show help if requested
                         "0.1");  // version string

      const std::string fname = args["<file>"].asString();
      const std::string out = args["<outfile>"].asString();
    */
    try {
        auto options = structopt::app("geoparser").parse<Options>(argc, argv);
        std::string fname = options.infile;
        std::string out = options.outfile.value();
        Options::ParseType pt = options.parse.value();
        fmt::print("Running on geometry file {}\n", fname);

        auto rows = extractRows(fname);

        auto writeFile = [&](auto&& v) {
            std::ofstream file;
            file.open(out);
            if (!file.is_open()) {
                spdlog::error("Failed to open file {}", fname);
            }
            cereal::XMLOutputArchive archive(file);
            archive(v);
        };

        if (pt == Options::ParseType::points) {
            auto res = parsePos(rows);
            writeFile(res);
        } else {
            auto res = parseShapes(rows);
            writeFile(res);
        }

    } catch (structopt::exception& e) {
        std::cout << e.what() << "\n";
        std::cout << e.help();
    }

    return 0;
}
