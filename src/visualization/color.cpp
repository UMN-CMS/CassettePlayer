#include "visualization/color.h"

#include "config/config.h"

void Color::getColorPtrFromName(Configuration* c) {
    if (is_named) real_color = c->getColor(color_name);
}
