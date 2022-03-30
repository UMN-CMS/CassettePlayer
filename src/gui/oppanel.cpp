#include "oppanel.h"

#include <array>
#include <memory>
#include <string>
#include <wx/sizer.h>

std::string exec(const char* cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

template <typename F>
std::future<std::string> runCmd(const std::string& cmd, F callback) {
    auto cmdLambda = [cmd] { return exec(cmd.c_str()); };
    auto fut = std::async([cmdLambda, callback] {
        const std::string stdout = cmdLambda();
        return callback(stdout);
    });
    return fut;
}

OpEntryPanel::OpEntryPanel(wxWindow* parent, wxWindowID id, const wxPoint& pos,
                           const wxSize& size, long style)
    : wxPanel(parent, id, pos, size, wxDOUBLE_BORDER | style) {
    // SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_BACKGROUND
    // ));

    auto topsizer = new wxBoxSizer(wxVERTICAL);
    auto scansizer = new wxBoxSizer(wxHORIZONTAL);
    auto lower_buttons = new wxBoxSizer(wxHORIZONTAL);

    entry_box = new wxTextCtrl(this, OpPanelItems::ID_ENTRY_BOX);
    scan_button = new wxButton(this, OpPanelItems::ID_SCAN_BUTTON, "Scan");

    complete_button = new wxButton(this, OpPanelItems::ID_COMPLETE_BUTTON,
                                   "Complete Step");
    next_button = new wxButton(this, OpPanelItems::ID_NEXT_BUTTON, "Next");
    reset_button = new wxButton(this, OpPanelItems::ID_RESET_BUTTON, "Reset");
    lower_buttons->AddStretchSpacer(1);
    lower_buttons->Add(reset_button, 0, wxALL, 10);
    lower_buttons->Add(complete_button, 0, wxALL, 10);
    lower_buttons->Add(next_button, 0, wxALL, 10);

    scansizer->AddStretchSpacer(2);
    scansizer->Add(entry_box, 2, wxEXPAND, 10);
    scansizer->Add(scan_button, 1, wxEXPAND, 10);
    scansizer->AddStretchSpacer(2);

    instruction_message =
        new wxStaticText(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
                         wxALIGN_CENTRE_HORIZONTAL);
    topsizer->Add(instruction_message, 0, wxEXPAND | wxALL, 5);
    topsizer->AddStretchSpacer(4);
    topsizer->AddStretchSpacer(1);
    topsizer->Add(scansizer, 0, wxEXPAND , 20);
    topsizer->AddStretchSpacer(4);
    topsizer->Add(lower_buttons, 0, wxEXPAND , 20);
    SetSizerAndFit(topsizer);
    auto callback = [this](std::string s) {
        auto evt = new wxCommandEvent(wxEVT_TEXT);
        evt->SetString(s);
        evt->SetId(ID_ASYNC_COMPLETE_RECIEVED);
        wxQueueEvent(this, evt);
        return s;
    };
    Bind(
        wxEVT_BUTTON,
        [this, callback](wxCommandEvent& WXUNUSED(e)) {
            this->scan_future =
                runCmd("sleep 3; printf \"HELLO WORLD\"", callback);
            return 1;
        },
        ID_SCAN_BUTTON);
    Bind(
        wxEVT_TEXT,
        [this](wxCommandEvent& e) {
            spdlog::debug("Recieved text event {}", e.GetString());
            this->entry_box->SetValue(e.GetString());
        },
        ID_ASYNC_COMPLETE_RECIEVED);
}

void OpEntryPanel::setButtonsEnabled(bool enabled) {
    complete_button->Enable(enabled);
    next_button->Enable(enabled);
    reset_button->Enable(enabled);
}

void OpEntryPanel::setEntryEnabled(bool enabled) {
    scan_button->Enable(enabled);
    entry_box->Enable(enabled);
}

void OpEntryPanel::setNewInstruction(Instruction* i) {
    if (i == nullptr) {
        spdlog::debug(
            "Attempted to set nullptr instruction in operations panel");
    }
    instruction_message->SetLabel(i->toString());
    current_instruction = i;
    if (i->op_type == OpType::EMPTY) {
        entry_box->SetValue("");
        setButtonsEnabled(false);
        setEntryEnabled(false);
    } else {
        if (dm->isInstructionComplete(i)) {
            if (i->cas_slot) {
                auto id_o =
                    dm->opRecord().getCasSlotIdentifier(i->cas_slot.value());
                if (id_o) entry_box->SetValue(id_o.value().identifier);
            } else {
                entry_box->SetValue("");
            }
            setButtonsEnabled(false);
            setEntryEnabled(false);
        } else {
            if (i->cas_slot->isDistinct()) {
                setButtonsEnabled(true);
                setEntryEnabled(true);

            } else {
                entry_box->SetValue("");
                setButtonsEnabled(true);
                setEntryEnabled(false);
            }
        }
    }
}
void OpEntryPanel::resetAll() {}
void OpEntryPanel::onResetButton(wxCommandEvent& WXUNUSED(e)) {}

std::pair<Instruction*, std::string> OpEntryPanel::getCurrentValues() const {
    if (!current_instruction) {
        spdlog::error("Op panel has no current instruction");
        return {nullptr, ""};
    }
    return {current_instruction, entry_box->GetValue().ToStdString()};
}
