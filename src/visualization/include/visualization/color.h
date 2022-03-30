#pragma once

#include <array>
#include <cereal/types/array.hpp>

class Configuration;

class Color {
   private:
    std::array<unsigned char, 4> vals = {255, 0, 0, 255};

    bool is_named = false;
    std::string color_name;
    Color* real_color;

   public:
    Color() = default;
  Color(std::string s) : is_named{true},color_name{std::move(s)} {}
    Color(std::array<unsigned char, 4> arr)
        : vals{std::move(arr)}, is_named{false} {}
    Color(unsigned int hex) { setRGB(hex); }

  void setRGB(unsigned int val) { setRGBA((val << 8) | 0xFF); }
    void setRGBA(unsigned int val) {
        is_named = false;
        for (std::size_t i = 0; i < 4; ++i) {
            vals[i] = (val >> (8 * (3 - i)) & 0xFF);
        }
    }

    void getColorPtrFromName(Configuration* c);

    inline auto& r() { return vals[0]; }
    inline auto& g() { return vals[1]; }
    inline auto& b() { return vals[2]; }
    inline auto& a() { return vals[3]; }

    inline auto& operator[](std::size_t i) { return vals[i]; }

    inline auto r() const { return vals[0]; }
    inline auto g() const { return vals[1]; }
    inline auto b() const { return vals[2]; }
    inline auto a() const { return vals[3]; }

    inline auto operator[](std::size_t i) const { return vals[i]; }

    template <typename Archive>
    void serialize(Archive& ar) {
        ar(is_named);
        if (is_named)
            ar(color_name);
        else
            ar(vals);
    }
};
