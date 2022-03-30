#pragma once

#include <wx/htmllbox.h>

#include "casdm.h"

class OpRecordPanel : public wxHtmlListBox {
   private:
    DataManager* dm;

   public:
    void setDataManager(DataManager* d) { dm = d; }
    OpRecordPanel(wxWindow* parent, wxWindowID id = wxID_ANY,
                  const wxPoint& pos = wxDefaultPosition,
                  const wxSize& size = wxDefaultSize, long style = 0,
                  const wxString& name = wxVListBoxNameStr);
    wxString OnGetItem(size_t n) const override;
    void Update() override;
};
