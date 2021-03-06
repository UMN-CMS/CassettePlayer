#include "visualization.h"

#include <fmt/core.h>
#include <wx/log.h>
#include <wx/stdpaths.h>
#include <wx/wx.h>

#include <cmath>

#include "drawables.h"

//IMPLEMENT_DYNAMIC_CLASS( VisualizationClick,wxEvent )
wxDEFINE_EVENT(VIS_FRAME_LEFT_DOWN, VisualizationClickEvent);
wxDEFINE_EVENT(VIS_FRAME_COMPONENT_SELECTED, ComponentSelectedEvent);


BEGIN_EVENT_TABLE(VisualizationCanvas, wxPanel)
EVT_PAINT(VisualizationCanvas::OnPaint)
EVT_CHAR(VisualizationCanvas::OnKey)
EVT_MOUSE_EVENTS(VisualizationCanvas::OnMouse)
END_EVENT_TABLE()


std::vector<std::pair<LayerKeyType, std::function<bool(DrawableCasElement*)>>>
createMap() {
    std::vector<
        std::pair<LayerKeyType, std::function<bool(DrawableCasElement*)>>>
        ret;

    ret.push_back({"Wagon", [](const DrawableCasElement* d) {
                       return d->cas_slot.isType<SlotWagon>();
                   }});
    ret.push_back({"Engine", [](const DrawableCasElement* d) {
                       return d->cas_slot.isType<SlotEngine>();
                   }});
    ret.push_back({"Deported DCDC", [](const DrawableCasElement* d) {
                       return d->cas_slot.isType<SlotDepDCDC>();
                   }});
    ret.push_back({"Other Types", [](const DrawableCasElement* d) {
                       return d->cas_slot.isType<SlotNull>();
                   }});
    ret.push_back({"Module", [](const DrawableCasElement* d) {
                       return d->cas_slot.isType<SlotModule>();
                   }});
    return ret;
}

template <typename T>
auto getFromPair(const std::string& s, T&& cont) {
    return std::find_if(cont.begin(), cont.end(),
                        [&](auto&& x) { return s == x.first; })
        ->second;
}

VisualizationCanvas::VisualizationCanvas(wxWindow* parent, wxWindowID id,
                                         const wxPoint& pos, const wxSize& size)
    : wxPanel(parent, id, pos, size) {
    id_map = createMap();
    for (const auto& p : id_map) {
        arrangement.push_back({p.first, true});
    }
    /*
    VisualizationClick event(VIS_FRAME_LEFT_DOWN, GetId(), {0,0});
        event.SetEventObject(this);
        bool ok = ProcessWindowEvent(event);
        spdlog::debug("Processed: {}", ok);
    */

}

void VisualizationCanvas::render(wxGraphicsContext* gc, wxDC& dc) {
    if (!gc) return;
    dc.SetBackground(*wxLIGHT_GREY);
    dc.Clear();
    gc->SetFont(*wxNORMAL_FONT, *wxWHITE);
    gc->GetSize(&width, &height);
    gc->SetTransform(gc->CreateMatrix(getTransformation(width, height)));
    gc->SetPen(*wxRED_PEN);
    auto ptr = gc;
    for (const auto& d_el_pair : drawn_elements) {
        const auto& s = d_el_pair.first;
        s->draw(ptr, d_el_pair.second);
    }
    if (draw_mouse_pos) {
        gc->SetTransform(gc->CreateMatrix());
        double extw, exth;
        std::string pos;
        if (mouse_or_center) {
            pos = fmt::format("({:.1f},{:.1f})", mouse_x, mouse_y);
        } else {
            pos = fmt::format("({:.1f},{:.1f})", x, y);
        }
        gc->GetTextExtent(pos, &extw, &exth);
        gc->DrawText(pos, width - extw, height - exth);
    }
}

void VisualizationCanvas::OnPaint(wxPaintEvent& WXUNUSED(e)) {
    wxPaintDC dc(this);
    auto gc = std::unique_ptr<wxGraphicsContext>(wxGraphicsContext::Create(dc));
    render(gc.get(), dc);
}

wxAffineMatrix2D VisualizationCanvas::getTransformation(
    float width, float WXUNUSED(height)) const {
    wxAffineMatrix2D mat{};
    mat.Scale(1, -1);
    mat.Translate(-(x - width / 2), -(y + height / 2));
    mat.Rotate(angle);
    mat.Scale(zoom, zoom);
    return mat;
}


void VisualizationCanvas::OnMouse(wxMouseEvent& e) {
    Refresh();
    float cur_x = e.GetX();
    float cur_y = e.GetY();
    if (frozen) return;
    setMousePosition(cur_x, cur_y);
    if (e.LeftIsDown()) {
        rot_active = e.ControlDown();
        if (e.Dragging()) {
            float delta_x = cur_x - prev_x;
            float delta_y = cur_y - prev_y;
            if (rot_active) {
                if (std::abs(cur_x - rot_origin_x) > 0.01 &&
                    std::abs(prev_x - rot_origin_x))

                    angle += std::atan((cur_y - rot_origin_y) /
                                       (cur_x - rot_origin_x)) -
                             std::atan((prev_y - rot_origin_y) /
                                       (prev_x - rot_origin_x));
            } else {
                x = x - delta_x;  // std::clamp<float>(x+delta_x,min_x,max_x);
                y = y + delta_y;  // std::clamp<float>(y+delta_y,min_y,max_y);
            }
        } else {
            rot_origin_x = cur_x;
            rot_origin_y = cur_y;
        }

        prev_x = cur_x;
        prev_y = cur_y;
    }
    if (e.LeftDown()) {
    }
    if (e.LeftUp() && !e.Dragging()) {
        VisualizationClickEvent event(VIS_FRAME_LEFT_DOWN, GetId(), {cur_x, cur_y});
        event.SetEventObject(this);
        wxPostEvent(GetParent(), event);
        auto hits = hit(cur_x, cur_y);
        if (false && std::size(hits) == 1) {

            // hit_dialog.ShowModal();
        } else if (std::size(hits) >= 1) {
          ComponentSelectedEvent event(VIS_FRAME_COMPONENT_SELECTED, GetId(), hits.back()->cas_slot);
          event.SetEventObject(this);
          ProcessWindowEvent(event);
        }
    }
    zoom = std::clamp<float>(zoom + e.GetWheelRotation() / 1000.0f, 0.3, 10.0f);
    Refresh();
}

void VisualizationCanvas::OnKey(wxKeyEvent& WXUNUSED(e)) { Refresh(); }

std::vector<DrawableCasElement*> VisualizationCanvas::hit(double x, double y) {
    wxAffineMatrix2D mat = getTransformation(width, height);
    mat.Invert();
    mat.TransformPoint(&x, &y);
    std::vector<DrawableCasElement*> ret;
    for (const auto& element_pair : drawn_elements) {
        const auto& element = element_pair.first;
        if (!element->interactable) continue;
        if (element->isWithin(x, y)) {
            makeSelected(element);
            spdlog::debug("{}", element->cas_slot.toString());
            ret.push_back(element);
        } else {
            element->selected = false;
        }
    }
    return ret;
}

void VisualizationCanvas::repopulateDrawn() {
    drawn_elements.clear();
    for (const auto& element_pair : elements) {
        // const auto& element = element_pair.first;
        drawn_elements.push_back(element_pair);
    }
}

void VisualizationCanvas::setMousePosition(double x, double y) {
    wxAffineMatrix2D mat = getTransformation(width, height);
    // spdlog::debug("Untransformed mouse pos ({},{}) ", x,y);
    mat.Invert();
    mat.TransformPoint(&x, &y);
    mouse_x = x;
    mouse_y = y;
}

void VisualizationCanvas::centerOnCenter() {
    x = 0.5 * (max_x + min_x);
    y = 0.5 * (max_y + min_y);
}

void VisualizationCanvas::applyCurrentLayers() {
    for (const auto& p : arrangement) {
        if (!p.second) {
            filterIf(getFromPair(p.first, id_map));
        }
    }
}
void VisualizationCanvas::orderLayers() {
    for (const auto& p : arrangement) {
        std::stable_partition(
            drawn_elements.begin(), drawn_elements.end(), [this, &p](auto&& x) {
                return getFromPair(p.first, this->id_map)(x.first);
            });
    }
}
void VisualizationCanvas::setArrangment(
    std::vector<std::pair<LayerKeyType, bool>> arr) {
    for (auto it = arr.rbegin(); it != arr.rend(); ++it) {
        auto found =
            std::find_if(arrangement.begin(), arrangement.end(),
                         [y = it->first](auto&& x) { return x.first == y; });
        found->second = it->second;
        std::rotate(arrangement.begin(), found, found + 1);
    }
    applyAndOrder();
}

void VisualizationCanvas::applyAndOrder() {
    repopulateDrawn();
    applyCurrentLayers();
    orderLayers();
}

std::vector<LayerKeyType> VisualizationCanvas::getAllLayers() const {
    std::vector<LayerKeyType> ret;
    for (const auto& p : id_map) {
        ret.push_back(p.first);
    }
    return ret;
}

std::vector<std::pair<LayerKeyType, bool>> VisualizationCanvas::getArrangment()
    const {
    return arrangement;
}

void VisualizationCanvas::makeFocus(const CasSlot& c) {
    auto f = std::find_if(elements.begin(), elements.end(),
                          [c](auto&& x) { return x.first->cas_slot == c; });
    if (f == std::end(elements)) {
        spdlog::error(
            "Attempting to focus on cassette slot that is not present on "
            "current visualization");
        return;
    }
    DrawableCasElement* found_el = f->first;
    if (found_el) {
        makeFocus(found_el);
    } else {
        spdlog::error("Found nullptr cas element when looking for slot {}",
                      c.toString());
    }
}

void VisualizationCanvas::makeFocus(DrawableCasElement* el) {
    spdlog::debug("Setting focus to {}", el->toString());
    x = el->pos.x();
    y = el->pos.y();
    spdlog::debug("Set location to ({},{})", x, y, angle);
}
void VisualizationCanvas::makeFocus(std::size_t i) {
    if (i < std::size(drawn_elements)) {
        makeFocus(drawn_elements[i].first);
    } else {
        spdlog::error(
            "Attempted to access nonexistent index {} of elements in main "
            "visualization canvas",
            i);
    }
}

void VisualizationCanvas::makeSelected(DrawableCasElement* d) {

    for (DrawPair& pair : elements) {
        pair.second.is_selected = (d == pair.first);
    }
    for (DrawPair& pair : drawn_elements) {
        pair.second.is_selected = (d == pair.first);
    }
}

void VisualizationCanvas::makeSelected(const CasSlot& c) {

    auto f = std::find_if(elements.begin(), elements.end(),
                          [c](auto&& x) { return x.first->cas_slot == c; });
    if (f == std::end(elements)) {
        spdlog::error(
            "Attempting to select cassette slot that is not present on "
            "current visualization");
        return;
    }
    DrawableCasElement* found_el = f->first;
    if (found_el) {
        makeSelected(found_el);
    } else {
        spdlog::error("Found nullptr cas element when looking for slot {}",
                      c.toString());
    }
}
