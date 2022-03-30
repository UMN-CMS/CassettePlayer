#pragma once

#include <wx/button.h>
#include <wx/panel.h>
#include <wx/statbox.h>
#include <wx/stattext.h>
#include <wx/textctrl.h>

#include <future>

#include "casdm.h"

enum OpPanelItems {
    ID_ENTRY_BOX = 1200,
    ID_SCAN_BUTTON,
    ID_ASYNC_COMPLETE_RECIEVED,
    ID_COMPLETE_BUTTON,
    ID_RESET_BUTTON,
    ID_NEXT_BUTTON
};

class OpEntryPanel : public wxPanel {
   private:
    DataManager* dm;
    // bool enabled = true;
    // Instruction* ins;
    // std::optional<Operation> op;
    // std::string description = "";
    Instruction* current_instruction;
    std::string scancode;
    wxStaticText* instruction_message;
    wxTextCtrl* entry_box;
    wxButton* scan_button;
    wxButton* complete_button;
    wxButton* next_button;
    wxButton* reset_button;
    std::future<std::string> scan_future;

   public:
    void setDataManager(DataManager* d) { dm = d; }
    void setNewInstruction(Instruction* i);
    void setButtonsEnabled(bool enabled = true);
    void setEntryEnabled(bool enabled = true);
    void resetAll();
    void onResetButton(wxCommandEvent& e);
    OpEntryPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
                 const wxPoint& pos = wxDefaultPosition,
                 const wxSize& size = wxDefaultSize,
                 long style = wxTAB_TRAVERSAL);

    std::pair<Instruction*, std::string> getCurrentValues() const;
};
