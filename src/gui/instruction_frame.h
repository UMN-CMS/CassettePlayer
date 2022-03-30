#pragma once

#include <wx/checklst.h>
#include <wx/dataview.h>
#include <wx/frame.h>
#include <wx/gauge.h>
#include <wx/panel.h>
#include <wx/scrolwin.h>
#include <wx/settings.h>
#include <wx/sizer.h>
#include <wx/splitter.h>
#include <wx/statbox.h>

#include "casdm.h"
#include "instructdv.h"
#include "oppanel.h"
#include "oprecord.h"
#include "visframe.h"
#include "visualization/visualization.h"

class InstructionFrame : public wxFrame {
    DECLARE_EVENT_TABLE()
   private:
    DataManager* dm = nullptr;
    bool vis_frame_open = false;
    wxMenu* view_menu;

   protected:
    wxGauge* progress_gauge;

    wxSplitterWindow* main_split;
    wxPanel* instruct_panel;

    wxDataViewCtrl* instructions;
    wxPanel* sub_inst_panel;

    OpEntryPanel* current_instruct;

    wxPanel* top_bar;

    wxPanel* hist_panel;
    OpRecordPanel* op_record_panel;
    VisualizationCanvas* miniview;
    wxButton* user_name;

    VisualizationFrame* vis_frame;

    InstructionModel* subinst_model;
    wxDataViewCtrl* subinstructions;

    void setCurInstPanel(Instruction* ins);
    void setSubDVRoot(Instruction* ins);
    void setMiniView(Instruction* ins);

   public:
    void createMenuBar();
    void setDataManager(DataManager* d) { dm = d; }
    DataManager& getDM() { return *dm; }
    InstructionFrame(DataManager* dm, wxFrame* parent, wxWindowID id = wxID_ANY,
                     const wxString& title = "CassettePlayer",
                     const wxPoint& pos = wxDefaultPosition,
                     const wxSize& size = wxSize(1200, 700),
                     long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);

    VisualizationFrame* createVisualizationFrame();

    void onCloseVisFrame(wxCloseEvent& e);

    void onCreateVisualizationFrame(wxCommandEvent& e);

    void onClickMainInsItem(wxDataViewEvent& e);
    void onClickSubInsItem(wxDataViewEvent& e);

    void onMenuLayerSelector(wxCommandEvent& e);

    void onGotoComp(wxCommandEvent& e);

    void onCompleteButtonPressed(wxCommandEvent& e);
    void onNextButtonPressed(wxCommandEvent& e);

    void onMenuAbout(wxCommandEvent& e);
    void onMenuOpenNewCassette(wxCommandEvent& e);
    void onMenuPrint(wxCommandEvent& e);
    void onMenuPrintSetup(wxCommandEvent& e);
    void onMenuSave(wxCommandEvent& e);
    void onMenuSaveAs(wxCommandEvent& e);
    void onMenuNew(wxCommandEvent& e);
    void onPartList(wxCommandEvent& e);
    void onCurrentInstruction(wxCommandEvent& e);
    void onNextInstruction(wxCommandEvent& e);

    void onViewSubassembly(wxCommandEvent& e);
    void onChangeUserPress(wxCommandEvent& e);
    void gotoCurrentInstructionInView();

    void onPrintPreview(wxCommandEvent& e);
    void onPrint(wxCommandEvent& e);

    ~InstructionFrame();
};
