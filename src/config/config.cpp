#include "config.h"

Color* Configuration::getColor(const std::string& name) const {
    return colors.at(name).get();
}
