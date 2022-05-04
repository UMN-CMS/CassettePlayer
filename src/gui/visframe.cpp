#include "visframe.h"

#include <wx/rearrangectrl.h>

VisualizationFrame::VisualizationFrame(wxFrame* parent, wxWindowID id,
                                       const wxString& title,
                                       const wxPoint& pos, const wxSize& size,
                                       long style)
    : wxFrame(parent, id, title, pos, size, style) {
    spdlog::debug("Constructing visualization frame");
    vc = new VisualizationCanvas(this);
}

void VisualizationFrame::initializeGeometry() {
    if (dm != nullptr) {
        spdlog::debug("Initializing geometry for geometry canvas {}",
                      (void*)this);
        //    vc->setAngle(-3.1415/3 * 2);
        vc->mouse_or_center = false;
        auto v = dm->getVisForCurrentCas();
        vc->setElements(v);
    } else {
        spdlog::error(
            "Attempted to initialize geometry for canvas {} with nullptr "
            "DataManager",
            (void*)this);
    }
    dm->updateAllPresentForCas();
}

void VisualizationFrame::openLayerSelector() {
    wxArrayString opts;
    wxArrayInt order;
    const auto pairs = vc->getArrangment();
    spdlog::debug("Pairs are {}", pairs);
    for (std::size_t i = 0; i < pairs.size(); ++i) {
        int t = i;
        opts.Add(pairs[i].first);
        order.Add((pairs[i].second) ? t : ~t);
    }
    wxRearrangeDialog dialog(this, "Display and reorder layers.",
                             "Layers and Ordering", order, opts);
    int ret = dialog.ShowModal();
    if (ret == wxID_OK) {
        auto newpairs = pairs;
        wxArrayInt onew = dialog.GetOrder();
        for (std::size_t i = 0; i < onew.size(); ++i) {
            auto prev = pairs[(onew[i] < 0) ? ~onew[i] : onew[i]];
            prev.second = (onew[i] >= 0);
            newpairs[i] = prev;
        }
        spdlog::debug("New set is {}", newpairs);
        vc->setArrangment(newpairs);
    }
}

void VisualizationFrame::gotoComp(const CasSlot& c) { vc->makeSelected(c); }

void VisualizationFrame::AddPendingEvent(const wxEvent& e){
  spdlog::debug("EVENT ID {} , command {} ", e.GetId(), e.IsCommandEvent());
  wxEvtHandler::AddPendingEvent(e);
}
