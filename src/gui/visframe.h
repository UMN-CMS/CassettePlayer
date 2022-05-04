#include "casdm.h"
#include "visualization/visualization.h"

class VisualizationFrame : public wxFrame {
   private:
    VisualizationCanvas* vc;
    DataManager* dm = nullptr;

   public:
    VisualizationCanvas* getVC() { return vc; }
    void initializeGeometry();
    void setDataManager(DataManager* d) { dm = d; }
    DataManager& getDM() { return *dm; }
    VisualizationFrame(wxFrame* parent, wxWindowID id = wxID_ANY,
                       const wxString& title = "Cassette Visualization",
                       const wxPoint& pos = wxDefaultPosition,
                       const wxSize& size = wxSize(800, 600),
                       long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    void openLayerSelector();
    void gotoComp(const CasSlot& c);
  void AddPendingEvent(const wxEvent&) override;
};
