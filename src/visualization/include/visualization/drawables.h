#pragma once

#include <cereal/types/array.hpp>
#include <cereal/types/vector.hpp>
#include <optional>
#include <utility>

#include "color.h"
#include "core/slots.h"
#include "vmath/vmath.h"

class Configuration;

class wxGraphicsContext;

using DrawCanvasType = wxGraphicsContext;
struct VisualizationOptions {
    bool is_selected = false;
    bool respect_present = true;
};

/*
struct Drawable {
    void draw(DrawCanvasType* gc, const VisualizationOptions&) const;
    void draw(DrawCanvasType* gc) const;
    bool isWithin(float x, float y) const;
};
*/

struct DrawableOptions {
    bool present = false;
    bool fill_poly = true;
};

class StyledPolygon {
   protected:
    PointVec points;

   public:
    // virtual DrawableCasElement* clone() const;
    Color color;
    float line_width = 1;
    StyledPolygon(PointVec p) : points{std::move(p)} {}
    StyledPolygon(PointVec p, Color c) : StyledPolygon(std::move(p)) {
        color = std::move(c);
    }
    StyledPolygon() = default;

    void draw(DrawCanvasType* gc, const VisualizationOptions&,
              const DrawableOptions& o) const;
    bool isWithin(float x, float y) const;
    ~StyledPolygon() = default;

    template <typename Archive>
    void serialize(Archive& ar) {
        ar(points, color);
    }
};

struct Drawable {
    std::vector<StyledPolygon> shapes;
    Drawable() = default;
    Drawable(std::vector<StyledPolygon> s) : shapes{std::move(s)} {}

    void syncColorsToConfig(Configuration* c);

    template <typename Archive>
    void serialize(Archive& ar) {
        ar(shapes);
    }
    void draw(DrawCanvasType* gc, const VisualizationOptions&,
              const DrawableOptions& o) const;
    bool isWithin(float x, float y) const;
    Configuration* conf;
};

class DrawableCasElement {
   public:
    Drawable drawable;
    PositionInfo pos;
    // The cassette component this drawabale draws
    CasSlot cas_slot = CasSlot(SlotNull());
    DrawableOptions dopts;

    // Optionaly, the external element insterted into this slot
    std::optional<ExternalComponent> ext_comp;


    bool active = false;
    bool selected = false;
    bool interactable = true;

    void setPresent(bool present = true) { dopts.present = present; }

    DrawableCasElement() = default;
    DrawableCasElement(Drawable p);
    DrawableCasElement(float _x, float _y, Drawable p);
    DrawableCasElement(float _x, float _y, float angle, Drawable p);
    DrawableCasElement(PositionInfo p, Drawable sp);


    void draw(DrawCanvasType* gc, const VisualizationOptions&) const;
    void draw(DrawCanvasType* gc) const;
    bool isWithin(float x, float y) const;
    ~DrawableCasElement() = default;
    std::string toString() const;
    // virtual DrawableCasElement* clone() const = 0;
};
