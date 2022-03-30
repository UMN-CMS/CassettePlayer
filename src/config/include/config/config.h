#pragma once

#include <array>
#include <cereal/cereal.hpp>

#include "visualization/color.h"

class Configuration {
   private:
    std::unordered_map<std::string, std::unique_ptr<Color>> colors;

   public:
    Color* getColor(const std::string& s) const;
};
