#pragma once

#include <spdlog/spdlog.h>

#include <cereal/cereal.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>
#include <cmath>
#include <functional>
#include <utility>

using Point = std::pair<float, float>;
using PointVec = std::vector<std::pair<float, float>>;

template <std::size_t N>
using PointArr = std::array<std::pair<float, float>, N>;

struct PositionInfo {
    Point p;
    float an;
    PositionInfo(Point p, float an) : p{std::move(p)}, an{an} {}
    PositionInfo() = default;
    template <typename Archive>
    void serialize(Archive& ar) {
        ar(p, an);
    }
    float& x() { return p.first; }
    float& y() { return p.second; }
    float& angle() { return an; }
    float x() const { return p.first; }
    float y() const { return p.second; }
    float angle() const { return an; }
};

template <typename T>
T operator+(const T& l, const T& r) {
    return {std::get<0>(l) + std::get<0>(r), std::get<1>(l) + std::get<1>(r)};
}

template <typename T>
T operator-(const T& l) {
    return {-std::get<0>(l), -std::get<1>(l)};
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<std::remove_reference_t<
                     decltype(std::get<0>(std::declval<T>()))>>,
                 T>
operator-(const T& l, const T& r) {
    return l + (-r);
}

template <typename T>
float operator*(const T& l, const T& r) {
    return std::get<0>(l) * std::get<0>(r) + std::get<1>(l) * std::get<1>(r);
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<std::remove_reference_t<
                     decltype(std::get<0>(std::declval<T>()))>>,
                 T>
operator*(const T& l, float r) {
    return {std::get<0>(l) * r, std::get<1>(l) * r};
}

template <typename T>
std::enable_if_t<std::is_floating_point_v<std::remove_reference_t<
                     decltype(std::get<0>(std::declval<T>()))>>,
                 T>
operator*(float r, const T& l) {
    return l * r;
}

template <typename T>
T operator/(const T& l, float r) {
    return {std::get<0>(l) / r, std::get<1>(l) / r};
}

template <typename T>
float norm(const T& p) {
    return std::sqrt(p * p);
}

template <typename T>
bool pointInPolygon(T&& vert, float testx, float testy) {
    using std::get;
    std::size_t i, j;
    bool c = false;
    for (i = 0, j = vert.size() - 1; i < vert.size(); j = i++) {
        if (((get<1>(vert[i]) > testy) != (get<1>(vert[j]) > testy)) &&
            (testx < (get<0>(vert[j]) - get<0>(vert[i])) *
                             (testy - get<1>(vert[i])) /
                             (get<1>(vert[j]) - get<1>(vert[i])) +
                         get<0>(vert[i])))
            c = !c;
    }
    return c;
}

template <typename T>
using ElementType = typename std::remove_reference_t<std::remove_cv_t<T>>;

template <typename T, typename U>
bool pointInPolygon(T&& vert, const U& point) {
    return pointInPolygon<T>(std::forward<T>(vert), std::get<0>(point),
                             std::get<1>(point));
}

template <typename T, typename U, typename F>
constexpr ElementType<U> applyTo(T&& appl, U&& arr, F&& func) {
    typename std::remove_reference_t<std::remove_cv_t<U>> ret = arr;
    for (auto& thing : ret) {
        thing = func(appl, thing);
    }
    return ret;
}

template <typename T, typename U>
constexpr ElementType<U> applyMatrix(T&& appl, U&& arr) {
    return applyTo(std::forward<T>(appl), std::forward<U>(arr),
                   [](auto&& appl, auto&& p) {
                       double x = std::get<0>(p);
                       double y = std::get<1>(p);
                       appl.TransformPoint(&x, &y);
                       return typename ElementType<U>::value_type(x, y);
                   });
}

template <typename T>
T bisectDir(const T& p1, const T& p2, const T& p3) {
    T dir = -((p2 - p1) / norm(p2 - p1) + (p2 - p3) / norm(p2 - p3));
    return dir / norm(dir);
}

template <typename T, std::size_t N = 3>
T centerOfRegPolygon(const T& p1, const T& p2, const T& p3, float s) {
    static const float pi = 3.1415926;
    float sidel = s;
    spdlog::debug("Side length: {}", sidel);
    float to_center_l;
    if constexpr (N == 6) {
        to_center_l = s;

    } else {
        to_center_l =
            0.5f * std::sqrt(std::pow(sidel, 2) + std::cos(2 * pi / N));
    }
    spdlog::debug("To center l {}", to_center_l);
    T dir = -((p2 - p1) / norm(p2 - p1) + (p2 - p3) / norm(p2 - p3));
    return (dir / norm(dir) * to_center_l) + p2;
}

template <typename T, std::size_t N = 3>
float threePointAngle(const T& p1, const T& p2, const T& p3) {
    //    float dot = ((p2 - p1) * (p2 - p3));
    float dot = ((p1 - p2) * (p3 - p2));
    return std::acos(dot / (norm(p2 - p1) * norm(p2 - p3)));
}

template <typename T>
float trueAngleBetween(const T& p1, const T& p2) {
    float dot = p1 * p2;
    float det = p1.first * p2.second - p1.second * p2.first;
    float angle = std::atan2(det, dot);
    return angle;
}
