#pragma once

#include <wx/graphics.h>
#include <wx/wx.h>

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>

#include "core/slots.h"
#include "drawables.h"
// #include "geometry/vmath.h"
class VisualizationClick : public wxCommandEvent {
   public:
  VisualizationClick(wxEventType eventType, int winid, const Point& _pos)
        : wxCommandEvent(winid, eventType), pos(_pos) {
    spdlog::debug("Construction visclick, is command event {}", IsCommandEvent());
  }
    wxEvent* Clone() const override {
        spdlog::debug("Cloning vis click");
        return new VisualizationClick(*this);
    }
    const Point pos;
};

wxDECLARE_EVENT(VIS_FRAME_LEFT_DOWN, VisualizationClick);

enum Layer {
    STATIC = 1 << 0,
    PLACED = 1 << 1,
    UNPLACED = 1 << 2,
    ENGINES = 1 << 3,
    WAGONS = 1 << 4,
    MODULES = 1 << 5,
    DCDC = 1 << 6,
};

using LayerKeyType = std::string;

class VisualizationCanvas : public wxPanel {
    DECLARE_EVENT_TABLE()

   private:
    float prev_x = 0, prev_y = 0;
    float rot_origin_x = 0, rot_origin_y = 0;
    bool rot_active = 0;
    float x = 0, y = 0, angle = 0;
    float zoom = 1;
    bool mirror = false;

    float max_x = 0, max_y = 0, min_x = 0, min_y = 0;

    double mouse_x = 0, mouse_y = 0;
    bool rotatable = true;

    double width, height;

    wxAffineMatrix2D getTransformation(float width, float height) const;

    std::vector<DrawableCasElement*> hit(double x, double y);

    template <typename Functor>
    void filterIf(Functor f) {
        drawn_elements.erase(
            std::remove_if(drawn_elements.begin(), drawn_elements.end(),
                           [f](auto&& x) { return f(x.first); }),
            drawn_elements.end());
    }

    void applyCurrentLayers();
    void orderLayers();

    void repopulateDrawn();

    //    void setExtents();
    void centerOnCenter();

    using DrawPair = std::pair<DrawableCasElement*, VisualizationOptions>;

    decltype(std::declval<DrawPair>().first) GD(const DrawPair& d) {
        return d.first;
    }

    std::vector<
        std::pair<LayerKeyType, std::function<bool(DrawableCasElement*)>>>
        id_map;
    std::vector<std::pair<LayerKeyType, bool>> arrangement;

    std::vector<DrawPair> elements;
    std::vector<DrawPair> drawn_elements;

   public:
    void setAngle(float a) { angle = a; }
    bool frozen = false;
    bool draw_mouse_pos = true;
    bool mouse_or_center = true;

    std::vector<LayerKeyType> getAllLayers() const;
    std::vector<std::pair<LayerKeyType, bool>> getArrangment() const;
    void applyAndOrder();
    void setArrangment(std::vector<std::pair<LayerKeyType, bool>> arr);

    void setMousePosition(double x, double y);

    void makeVisible(const CasSlot& c);
    void centerOn(const CasSlot& c);

    void makeSelected(const CasSlot& c);
    void makeSelected(DrawableCasElement* c);

    void makeFocus(const CasSlot& c);
    void makeFocus(std::size_t i);
    void makeFocus(DrawableCasElement* el);

    void addElements(std::vector<DrawableCasElement*> new_elements) {
        spdlog::debug("Adding {} elements to visualization",
                      std::size(new_elements));
        std::transform(new_elements.begin(), new_elements.end(),
                       std::back_inserter(elements), [](DrawableCasElement* d) {
                           return DrawPair{d, VisualizationOptions()};
                       });
        repopulateDrawn();
        applyAndOrder();
    }
    void setElements(std::vector<DrawableCasElement*> new_elements) {
        spdlog::debug("Setting {} elements to visualization",
                      std::size(new_elements));
        elements.clear();
        std::transform(new_elements.begin(), new_elements.end(),
                       std::back_inserter(elements), [](DrawableCasElement* d) {
                           return DrawPair{d, VisualizationOptions()};
                       });
        repopulateDrawn();
        applyAndOrder();
    }

    //    std::vector<DrawableCasElement*> other;

    VisualizationCanvas(wxWindow* parent, wxWindowID id = wxID_ANY,
                        const wxPoint& pos = wxDefaultPosition,
                        const wxSize& size = wxDefaultSize);

    void OnKey(wxKeyEvent& e);
    void OnMouse(wxMouseEvent& e);
    void OnPaint(wxPaintEvent& e);
    void render(wxGraphicsContext* gc, wxDC& dc);
};
