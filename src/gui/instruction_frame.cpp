#include "instruction_frame.h"

#include <fmt/ranges.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>
#include <wx/print.h>
#include <wx/stattext.h>

#include <cstdlib>
#include <future>
#include <type_traits>

#include "AppInfo.h"
#include "common.h"

class Printer : public wxPrintout {
   public:
    VisualizationFrame* vis_frame;
    bool OnPrintPage(int WXUNUSED(n)) override {
      //  FitThisSizeToPaper(wxSize(700,1000));
        auto gc = std::unique_ptr<wxGraphicsContext>(
            wxGraphicsContext::Create(GetDC()));
        vis_frame->getVC()->render(gc.get(), *GetDC());
        return true;
    }
};

enum MenuItems {
    ID_OPEN_VIS = 1100,
    ID_OPEN_LAYER_SELECTOR,
    ID_GOTO_ITEM,
    ID_CHANGE_USER,
    ID_NEXT_INSTRUCTION,
    ID_PART_LIST,
    ID_CURRENT_INSTRUCTION,
    ID_REPORT_ERROR,
    ID_VIEW_SUBASSEMBLY
};
enum Frames { ID_VIS_FRAME = 1000, ID_MAIN_INS_DV, ID_SUB_INS_DV };

BEGIN_EVENT_TABLE(InstructionFrame, wxFrame)
EVT_MENU(wxID_ABOUT, InstructionFrame::onMenuAbout)
EVT_MENU(wxID_NEW, InstructionFrame::onMenuNew)
EVT_MENU(wxID_SAVE, InstructionFrame::onMenuSave)
EVT_MENU(wxID_SAVEAS, InstructionFrame::onMenuSaveAs)
EVT_MENU(ID_GOTO_ITEM, InstructionFrame::onGotoComp)
EVT_MENU(ID_VIEW_SUBASSEMBLY, InstructionFrame::onViewSubassembly)
EVT_MENU(ID_OPEN_LAYER_SELECTOR, InstructionFrame::onMenuLayerSelector)
EVT_MENU(wxID_PRINT, InstructionFrame::onMenuPrint)
EVT_MENU(wxID_PRINT_SETUP, InstructionFrame::onMenuPrintSetup)
EVT_MENU(ID_OPEN_VIS, InstructionFrame::onCreateVisualizationFrame)
EVT_MENU(ID_CHANGE_USER, InstructionFrame::onChangeUserPress)
EVT_BUTTON(ID_CHANGE_USER, InstructionFrame::onChangeUserPress)

EVT_MENU(wxID_PRINT, InstructionFrame::onPrint)
EVT_MENU(wxID_PREVIEW, InstructionFrame::onPrintPreview)

EVT_MENU(ID_PART_LIST, InstructionFrame::onPartList)
EVT_MENU(ID_CURRENT_INSTRUCTION, InstructionFrame::onCurrentInstruction)
EVT_MENU(ID_NEXT_INSTRUCTION, InstructionFrame::onNextInstruction)

EVT_BUTTON(ID_COMPLETE_BUTTON, InstructionFrame::onCompleteButtonPressed)
EVT_BUTTON(ID_NEXT_BUTTON, InstructionFrame::onNextButtonPressed)

EVT_DATAVIEW_SELECTION_CHANGED(ID_MAIN_INS_DV,
                               InstructionFrame::onClickMainInsItem)
EVT_DATAVIEW_SELECTION_CHANGED(ID_SUB_INS_DV,
                               InstructionFrame::onClickSubInsItem)
END_EVENT_TABLE()

void InstructionFrame::gotoCurrentInstructionInView() {
    auto ins = toInstruction(instructions->GetSelection());
    if (ins && vis_frame_open) {
      spdlog::debug("Going to instruction {}", (void*) ins);
        auto c_o = ins->cas_slot;
        if (c_o) {
            vis_frame->gotoComp(c_o.value());
            vis_frame->Refresh();
        } else {
            spdlog::debug("Attempted to goto instruction with empty slot");
        }
    } else {
        spdlog::debug("Attempted to goto bad component (nullptr DataViewItem)");
    }
}

void InstructionFrame::createMenuBar() {
    wxMenu* fileMenu = new wxMenu;
    wxMenu* editMenu = (wxMenu*)NULL;

    fileMenu->Append(wxID_NEW, _("&New Cassette File\tCtrl-N"));
    fileMenu->Append(wxID_OPEN, _("&Open Cassette File\tCtrl-O"));
    fileMenu->Append(wxID_CLOSE, _("&Close"));
    fileMenu->Append(wxID_SAVE, _("&Save\tCtrl-S"));
    fileMenu->Append(wxID_SAVEAS, _("Save &As\tShift-Ctrl-S"));
    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_PRINT, _("&Print\tCtrl-P"));
    fileMenu->Append(wxID_PRINT_SETUP, _("Print &Setup..."));
    fileMenu->Append(wxID_PREVIEW, _("Print Pre&view \tShift-Ctrl-P"));

    editMenu = new wxMenu();
    //  editMenu->Append(wxID_UNDO, _("&Undo"));
    //  editMenu->Append(wxID_REDO, _("&Redo"));
    editMenu->AppendSeparator();

    fileMenu->AppendSeparator();
    fileMenu->Append(wxID_EXIT, _("E&xit"));

    wxMenu* assembly_menu = new wxMenu;
    assembly_menu->Append(ID_PART_LIST, "&Part List\tCtrl-L");
    assembly_menu->Append(ID_VIEW_SUBASSEMBLY, "View &Subassembly\tCtrl-_");
    assembly_menu->Append(ID_CURRENT_INSTRUCTION, "&Show Current Step\tCtrl-C");
    assembly_menu->Append(ID_NEXT_INSTRUCTION, "&Next Step\tCtrl-N");
    assembly_menu->Append(ID_CHANGE_USER, "Change Current &User");

    view_menu = new wxMenu;
    view_menu->AppendCheckItem(ID_OPEN_VIS, _("&Show Visualization\tCtrl-I"));
    view_menu->Append(ID_OPEN_LAYER_SELECTOR,
                      _("&Layer Modifier\tCtrl-Shift-L"));
    fileMenu->AppendSeparator();
    view_menu->Append(ID_GOTO_ITEM, _("&GoTo Selected Component\tCtrl-G"));

    wxMenu* helpMenu = new wxMenu;
    helpMenu->Append(wxID_ABOUT, _("&About"));
    helpMenu->Append(wxID_HELP, _("&Manual"));
    helpMenu->Append(ID_REPORT_ERROR, _("&Report Bug"));
    helpMenu->Append(wxID_PREFERENCES, _("&Preferences"));

    wxMenuBar* menuBar = new wxMenuBar;

    menuBar->Append(fileMenu, _("&File"));
    menuBar->Append(editMenu, _("&Edit"));
    menuBar->Append(assembly_menu, _("&Assembly"));
    menuBar->Append(view_menu, _("&Visualization"));
    menuBar->Append(helpMenu, _("&Help"));
    SetMenuBar(menuBar);
}

InstructionFrame::InstructionFrame(DataManager* dm, wxFrame* parent,
                                   wxWindowID id, const wxString& title,
                                   const wxPoint& pos, const wxSize& size,
                                   long style)
    : wxFrame(parent, id, title, pos, size, style) {
    setDataManager(dm);
    SetSizeHints(wxDefaultSize, wxDefaultSize);

    wxBoxSizer* topsizer;
    topsizer = new wxBoxSizer(wxVERTICAL);

    wxBoxSizer* subtopsizer;
    subtopsizer = new wxBoxSizer(wxVERTICAL);

    wxPanel* top_bar = new wxPanel(this, wxID_ANY, wxDefaultPosition,
                                   wxSize(-1, 50), wxBORDER_SIMPLE);
    subtopsizer->Add(top_bar, 0, wxEXPAND | wxALL, 5);

    wxBoxSizer* top_bar_main_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto current_cas_name = new wxButton(
        top_bar, wxID_ANY, "Cas", wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    current_cas_name->SetLabelMarkup(
        fmt::format("<big>Currently Operating On Cassette {}</big>",
                    dm->current_cas.toString()));
    user_name = new wxButton(top_bar, ID_CHANGE_USER, dm->current_user,
                             wxDefaultPosition, wxDefaultSize, wxBORDER_NONE);
    top_bar_main_sizer->Add(current_cas_name, 0, wxCENTER | wxALL, 10);
    top_bar_main_sizer->AddStretchSpacer(1);
    top_bar_main_sizer->Add(user_name, 0, wxCENTER);
    top_bar->SetSizer(top_bar_main_sizer);

    wxBoxSizer* progbar;
    progbar = new wxBoxSizer(wxHORIZONTAL);

    progress_gauge = new wxGauge(this, wxID_ANY, 100, wxDefaultPosition,
                                 wxDefaultSize, wxGA_SMOOTH | wxGA_VERTICAL);
    progress_gauge->SetValue(30);
    progress_gauge->SetMaxSize(wxSize(5, -1));

    progbar->Add(progress_gauge, 0, wxALL | wxEXPAND, 5);

    main_split =
        new wxSplitterWindow(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
                             wxSP_3D | wxSP_LIVE_UPDATE);


    instruct_panel =
        new wxPanel(main_split, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxBoxSizer* ins_split;
    ins_split = new wxBoxSizer(wxHORIZONTAL);

    instructions =
        new wxDataViewCtrl(instruct_panel, ID_MAIN_INS_DV, wxDefaultPosition,
                           wxDefaultSize, wxHSCROLL | wxVSCROLL);

    spdlog::debug("Creating instruction model with data manager {}", (void*)dm);
    auto model = new InstructionModel(dm);
    model->dm = dm;
    instructions->AssociateModel(model);
    model->Initialize();
    model->DecRef();  // avoid memory leak !!
    instructions->AppendTextColumn("Step", 0, wxDATAVIEW_CELL_INERT, 450);
    auto tempcol =
        new wxDataViewColumn("Progress", new CustomProgressRenderer(), 1);

    instructions->AppendColumn(tempcol);

    wxBoxSizer* subinssplit;
    subinssplit = new wxBoxSizer(wxVERTICAL);

    sub_inst_panel =
        new wxPanel(instruct_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    sub_inst_panel->SetBackgroundColour(
        wxSystemSettings::GetColour(wxSYS_COLOUR_ACTIVEBORDER));

    wxBoxSizer* subinst_sizer;
    subinst_sizer = new wxBoxSizer(wxVERTICAL);

    wxArrayString subinstructionsChoices;
    subinstructions = new wxDataViewCtrl(sub_inst_panel, ID_SUB_INS_DV,
                                         wxDefaultPosition, wxDefaultSize, 0);

    subinstructions->AppendTextColumn("Step Name", 0, wxDATAVIEW_CELL_INERT,
                                      450);
    subinstructions->AppendColumn(
        new wxDataViewColumn("Progress", new CustomProgressRenderer(), 1));

    subinst_sizer->Add(subinstructions, 1, wxALL | wxEXPAND, 5);

    sub_inst_panel->SetSizer(subinst_sizer);
    sub_inst_panel->Layout();
    subinst_sizer->Fit(sub_inst_panel);
    subinssplit->Add(sub_inst_panel, 1, wxEXPAND | wxALL, 5);

    current_instruct = new OpEntryPanel(instruct_panel, wxID_ANY,
                                        wxDefaultPosition, wxDefaultSize);
    current_instruct->setDataManager(dm);

    subinssplit->Add(current_instruct, 1, wxEXPAND, 5);

    ins_split->Add(instructions, 2, wxEXPAND | wxALL, 5);
    ins_split->Add(subinssplit, 2, wxEXPAND, 5);

    instruct_panel->SetSizer(ins_split);
    instruct_panel->Layout();
    ins_split->Fit(instruct_panel);
    hist_panel =
        new wxPanel(main_split, wxID_ANY, wxDefaultPosition, wxDefaultSize);
    wxBoxSizer* lower_sizer;
    lower_sizer = new wxBoxSizer(wxHORIZONTAL);

    wxStaticBoxSizer* op_hist_sizer;
    op_hist_sizer = new wxStaticBoxSizer(
        new wxStaticBox(hist_panel, wxID_ANY, _("Operation History")),
        wxVERTICAL);

    op_record_panel = new OpRecordPanel(hist_panel, wxID_ANY, wxDefaultPosition,
                                        wxDefaultSize);
    op_record_panel->setDataManager(dm);
    op_record_panel->Update();
    instruct_panel->Update();

    op_hist_sizer->Add(op_record_panel, 1, wxEXPAND, 5);

    lower_sizer->Add(op_hist_sizer, 1, wxEXPAND, 5);

    auto miniview_container = new wxPanel(hist_panel, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBORDER_SIMPLE);
     auto mvcont = new wxBoxSizer(wxVERTICAL);
     miniview = new VisualizationCanvas(miniview_container, wxID_ANY);
     // miniview->frozen = false;
     // miniview->draw_mouse_pos = true;
     // miniview->mouse_or_center = true;
     mvcont->Add(miniview, 1, wxEXPAND, 0);


     miniview_container->SetSizer(mvcont);

    lower_sizer->Add(miniview_container, 1, wxEXPAND | wxALL, 5);

    hist_panel->SetSizer(lower_sizer);
    hist_panel->Layout();
    lower_sizer->Fit(hist_panel);
    main_split->SplitHorizontally(instruct_panel, hist_panel, 400);
    main_split->SetMinimumPaneSize(20);
    progbar->Add(main_split, 1, wxEXPAND, 5);
    subtopsizer->Add(progbar, 3, wxEXPAND, 5);
    topsizer->Add(subtopsizer, 3, wxEXPAND, 5);
    SetSizer(topsizer);
    Layout();
    createMenuBar();
    Centre(wxBOTH);
    //   Iconize();
}

InstructionFrame::~InstructionFrame() {}

void InstructionFrame::onCloseVisFrame(wxCloseEvent& e) {
    spdlog::debug("Visualization frame closed");
    vis_frame_open = false;
    vis_frame = nullptr;
    view_menu->Check(ID_OPEN_VIS, false);
    e.Skip();
}

void InstructionFrame::onCreateVisualizationFrame(wxCommandEvent& WXUNUSED(e)) {
    if (vis_frame_open) return;
    createVisualizationFrame();
    if (vis_frame != nullptr) {
        spdlog::debug("Visualization frame created");
    } else {
        spdlog::debug(
            "Could not find close window event because visualization frame is "
            "nullptr");
    }
}

void InstructionFrame::setCurInstPanel(Instruction* ins) {
    current_instruct->setNewInstruction(ins);
}

void InstructionFrame::setMiniView(Instruction* ins) {
    if (ins->cas_slot) {
        auto ptr = dm->visMgr().getElement(ins->cas_slot.value());
        if (ptr) {
            const std::vector<DrawableCasElement*> d = {ptr};
            miniview->setElements(d);
            miniview->makeFocus(int(0));
            miniview->Refresh();;
            // miniview->Update();
            int w,h;
            miniview->GetSize(&w,&h);
            //             spdlog::debug("Current size of miniview is ({},{})", w,h);
        } else {
            spdlog::error("Attemped to add nullptr to miniview");
        }
    }
}

void InstructionFrame::setSubDVRoot(Instruction* ins) {
    if (dm->insMgr().getDirectChildren(ins).empty()) {
        Instruction* temp = dm->insMgr().getOneParent(ins);
        if (temp) {
            ins = temp;
        }
    }
    auto subinst_model = new InstructionModel(dm, ins);
    subinstructions->AssociateModel(subinst_model);
    subinst_model->Initialize();
}

void InstructionFrame::onClickMainInsItem(wxDataViewEvent& e) {
    Instruction* ins = toInstruction(e.GetItem());
    spdlog::debug("Selected instruction at {}", (void*)ins);
    if (ins) {
        setSubDVRoot(ins);
        setCurInstPanel(ins);
        setMiniView(ins);
        gotoCurrentInstructionInView();
    }
}

void InstructionFrame::onClickSubInsItem(wxDataViewEvent& e) {
    Instruction* ins = toInstruction(e.GetItem());
    spdlog::debug("Selected instruction at {}", (void*)ins);
    if (ins) {
        setCurInstPanel(ins);
        setMiniView(ins);
    }
}

void InstructionFrame::onMenuLayerSelector(wxCommandEvent& WXUNUSED(e)) {
    if (vis_frame_open && vis_frame) {
        vis_frame->openLayerSelector();
    }
}

void InstructionFrame::onGotoComp(wxCommandEvent& WXUNUSED(e)) {
    gotoCurrentInstructionInView();
}

void InstructionFrame::onCompleteButtonPressed(wxCommandEvent& WXUNUSED(e)) {
    auto op_panel_cur_vals = current_instruct->getCurrentValues();
    if (dm->attemptCompleteInstruction(op_panel_cur_vals)) {
        Refresh();
        current_instruct->setNewInstruction(
            current_instruct->getCurrentValues().first);
        op_record_panel->Update();
    }
}
void InstructionFrame::onNextButtonPressed(wxCommandEvent& WXUNUSED(e)) {
    spdlog::debug("Pressed Next");
}

void InstructionFrame::onMenuAbout(wxCommandEvent& WXUNUSED(e)) {
    auto dialog = wxMessageDialog(
        this,
        fmt::format("Cassette builder version {}.{}.\nDeveloped by UMN "
                    "CMS.\nTo report bugs please contact {}",
                    App_VERSION_MAJOR, App_VERSION_MINOR, "EMAIL"),
        "About");
    dialog.Centre();
    dialog.ShowModal();
}

void InstructionFrame::onMenuOpenNewCassette(wxCommandEvent& WXUNUSED(e)) {}
void InstructionFrame::onMenuPrint(wxCommandEvent& WXUNUSED(e)) {}
void InstructionFrame::onMenuPrintSetup(wxCommandEvent& WXUNUSED(e)) {}
void InstructionFrame::onMenuNew(wxCommandEvent& WXUNUSED(e)) {}
void InstructionFrame::onMenuSave(wxCommandEvent& WXUNUSED(e)) {}
void InstructionFrame::onMenuSaveAs(wxCommandEvent& WXUNUSED(e)) {}

VisualizationFrame* InstructionFrame::createVisualizationFrame() {
    vis_frame = new VisualizationFrame(this, ID_VIS_FRAME);
    // vis_frame->Bind(wxEVT_CLOSE_WINDOW, [](wxCloseEvent& ev) { ev.Skip(); });
    vis_frame->Bind(wxEVT_CLOSE_WINDOW, &InstructionFrame::onCloseVisFrame,
                    this);
    view_menu->Check(ID_OPEN_VIS, true);

    vis_frame->setDataManager(dm);
    vis_frame->initializeGeometry();

    vis_frame->Show();
    vis_frame_open = true;

    return vis_frame;
}

void InstructionFrame::onPartList(wxCommandEvent& WXUNUSED(e)) {}
void InstructionFrame::onCurrentInstruction(wxCommandEvent& WXUNUSED(e)) {
    instructions->EnsureVisible(instructions->GetCurrentItem());
}
void InstructionFrame::onNextInstruction(wxCommandEvent& WXUNUSED(e)) {}

void InstructionFrame::onViewSubassembly(wxCommandEvent& WXUNUSED(e)) {
    auto subassembly_dialog =
        new wxDialog(this, wxID_ANY, "Subassembly", wxDefaultPosition,
                     wxSize(300,200));

    auto subview = new VisualizationCanvas(subassembly_dialog);

    std::vector<DrawableCasElement*> dce;
    Instruction* cur_ins = toInstruction(instructions->GetCurrentItem());
    spdlog::debug("Selected instruction at {}", (void*)cur_ins);
    if (!cur_ins) return;
    subview->frozen = true;
    subview->draw_mouse_pos = false;
    subview->mouse_or_center = false;
    //    subview->setAngle(dm->getRenderAngleForCas());

    for (Instruction* ins : dm->insMgr().getAllChildren(cur_ins)) {
        if (ins->cas_slot) {
            auto ptr = dm->visMgr().getElement(ins->cas_slot.value());
            if (ptr) {
                dce.push_back(ptr);
            } else {
                spdlog::error("Attemped to add nullptr to miniview");
            }
        }
    }
    subview->setElements(dce);
    subview->makeFocus(int(0));
    subview->Refresh();
    subassembly_dialog->Show();
    subassembly_dialog->Centre();
}

void InstructionFrame::onChangeUserPress(wxCommandEvent& WXUNUSED(e)) {
    auto dialog = wxTextEntryDialog(this, "Please enter the current operator",
                                    "Current User");
    if (dialog.ShowModal() == wxID_OK) {
        // We can be certain that this string contains letters only.
        wxString value = dialog.GetValue();
        dm->current_user = value.ToStdString();
        user_name->SetLabelMarkup(
            fmt::format("<big>Current User: {}</big>", dm->current_user));
        spdlog::debug("Changing current user to {}", dm->current_user);
        Layout();
    }
}

void InstructionFrame::onPrintPreview(wxCommandEvent& WXUNUSED(e)) {
    wxPrinter printer;
    Printer printout;
    printout.vis_frame = vis_frame;
    printer.Print(this, &printout, true);
}

void InstructionFrame::onPrint(wxCommandEvent& WXUNUSED(e)) {
    auto printer1 = new Printer;
    auto printer2 = new Printer;
    printer1->vis_frame = vis_frame;
    printer2->vis_frame = vis_frame;
    wxPrintPreview* preview = new wxPrintPreview(printer1, printer2);
    wxPreviewFrame* frame =
        new wxPreviewFrame(preview, this, "Demo Print Preview",
                           wxPoint(100, 100), wxSize(600, 650));
    frame->Centre(wxBOTH);
    frame->Initialize();
    frame->Show(true);
}
