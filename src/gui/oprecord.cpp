#include "oprecord.h"

OpRecordPanel::OpRecordPanel(wxWindow* parent, wxWindowID id,
                             const wxPoint& pos, const wxSize& size, long style,
                             const wxString& name)
    : wxHtmlListBox(parent, id, pos, size, style, name) {}

wxString OpRecordPanel::OnGetItem(size_t n) const {
    return dm->opRecord().getIthOp(n);
}

void OpRecordPanel::Update() {
    spdlog::debug("Updating record display to show {} records",
                  dm->opRecord().getNumOperations());
    SetItemCount(dm->opRecord().getNumOperations());
    wxHtmlListBox::Update();
}
