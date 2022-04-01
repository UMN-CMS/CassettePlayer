#include "drawables.h"

#include <wx/brush.h>
#include <wx/graphics.h>
#include <wx/log.h>
#include <wx/pen.h>

#include <array>
#include <memory>

void Drawable::draw(DrawCanvasType* gc, const VisualizationOptions& g,
                    const DrawableOptions& o) const {
    for (const StyledPolygon& p : shapes) {
        p.draw(gc, g, o);
    }
}

void DrawableCasElement::draw(DrawCanvasType* gc) const {
    draw(gc, VisualizationOptions());
}

void DrawableCasElement::draw(DrawCanvasType* gc,
                              const VisualizationOptions& g) const {
    auto transform = gc->GetTransform();
    gc->PushState();
    gc->SetTransform(gc->CreateMatrix());
    gc->ConcatTransform(transform);
    gc->Translate(pos.x(), pos.y());
    gc->Rotate(pos.angle());
    drawable.draw(gc, g, dopts);
    gc->PopState();
}
bool Drawable::isWithin(float x, float y) const {
    if (!shapes.empty()) {
        return shapes[0].isWithin(x, y);
    } else {
        return false;
    }
}
bool DrawableCasElement::isWithin(float x, float y) const {
    double xd = x, yd = y;
    wxAffineMatrix2D aff;
    aff.Rotate(-pos.angle());
    aff.Translate(-pos.x(), -pos.y());
    aff.TransformPoint(&xd, &yd);
    // spdlog::debug("Examining hit with point ({},{}) transformed to ({},{})",
    // x, y, xd, yd);
    return drawable.isWithin(xd, yd);
}

DrawableCasElement::DrawableCasElement(Drawable dr) : drawable{std::move(dr)} {}

DrawableCasElement::DrawableCasElement(float _x, float _y, Drawable p)

    : DrawableCasElement(std::move(p)) {
    pos.x() = _x;
    pos.y() = _y;
}
DrawableCasElement::DrawableCasElement(float _x, float _y, float a, Drawable p)

    : DrawableCasElement(_x, _y, std::move(p)) {
    pos.angle() = a;
}
DrawableCasElement::DrawableCasElement(PositionInfo p, Drawable sp)
    : DrawableCasElement(std::move(sp)) {
    pos = std::move(p);
}

void StyledPolygon::draw(DrawCanvasType* gc, const VisualizationOptions& g,
                         const DrawableOptions& d) const {
    auto c = wxColour(color.r(), color.g(), color.b(), color.a());

    long penstyle = wxPENSTYLE_SOLID;
    long brushstyle = wxBRUSHSTYLE_SOLID;
    if (g.respect_present && !d.present) {
        penstyle = wxPENSTYLE_SHORT_DASH;
        brushstyle=  wxBRUSHSTYLE_SOLID;
        auto newcolor =
            wxColour(color.r(), color.g(), color.b(), color.a() / 1.2);
        gc->SetBrush(wxBrush(newcolor, brushstyle));
    } else {
      gc->SetBrush(wxBrush(c, brushstyle));
    }

    if (g.is_selected) {
        gc->SetPen(wxPen(*wxBLUE, line_width, penstyle));
        gc->SetBrush(wxBrush(*wxYELLOW, brushstyle));
    } else {
        gc->SetPen(wxPen(*wxWHITE, line_width, penstyle));
    }
    wxGraphicsPath path = gc->CreatePath();
#define unpack(x) x.first, x.second
    path.MoveToPoint(unpack(points[0]));
    for (std::size_t i = 1; i < std::size(points); ++i) {
        path.AddLineToPoint(unpack(points[i]));
    }

#undef unpack
    path.CloseSubpath();
    //    path.AddCircle(x, y, 2);
    if (d.fill_poly) {
        gc->FillPath(path);
    }
    gc->StrokePath(path);
}
bool StyledPolygon::isWithin(float x, float y) const {
    return pointInPolygon(points, x, y);
}

std::string DrawableCasElement::toString() const {
    return fmt::format("Drawable pos=({},{},{})", pos.x(), pos.y(),
                       pos.angle());
}
