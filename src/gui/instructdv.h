#pragma once

#include <wx/dataview.h>
#include "core/instruction.h"
#include "casdm.h"
#include <wx/dc.h>
#include <unordered_set>

class CustomProgressRenderer : public wxDataViewCustomRenderer {
   public:
    explicit CustomProgressRenderer() : wxDataViewCustomRenderer("long") {}

    virtual bool Render(wxRect rect, wxDC* dc, int WXUNUSED(state)) override {
        double pct = (double)value / 100.0;
        wxRect bar = rect;
        bar.width = (int)(rect.width * pct);
        dc->SetPen(*wxTRANSPARENT_PEN);
        if (value != 100) {
            dc->SetBrush(*wxBLUE_BRUSH);
        } else {
            dc->SetBrush(*wxGREEN_BRUSH);
        }
        dc->DrawRectangle(bar);
        dc->SetBrush(*wxTRANSPARENT_BRUSH);
        dc->SetPen(*wxBLACK_PEN);
        dc->DrawRectangle(rect);
        return true;
    }

    virtual wxSize GetSize() const override {
        // return GetView()->FromDIP(wxSize(60, 12));
        return wxSize(-1, 12);
    }

    virtual bool SetValue(const wxVariant& v) override {
        value = v.GetLong();
        return true;
    }

    virtual bool GetValue(wxVariant& WXUNUSED(value)) const override {
        return true;
    }

   private:
    long value;
};

class InstructionModel : public wxDataViewModel {
   private:
    Instruction* root = nullptr;
    bool initialized = false;
    void registerAdditions(Instruction* parent);
  //    std::unordered_set<Instruction*> initialized;

   public:
    DataManager* dm = nullptr;
    void Initialize();
    bool IsContainer(const wxDataViewItem& item) const override;
    wxDataViewItem GetParent(const wxDataViewItem& item) const override;
    void GetValue(wxVariant& v, const wxDataViewItem& item,
                  unsigned int col) const override;
    unsigned int GetChildren(const wxDataViewItem& item,
                             wxDataViewItemArray&) const override;
    unsigned int GetColumnCount() const override;
    wxString GetColumnType(unsigned int column) const override;
    bool SetValue(const wxVariant& v, const wxDataViewItem& item,
                  unsigned int col) override;
    InstructionModel(DataManager* d, Instruction* r) : root{r}, dm{d} {}
    InstructionModel(DataManager* d) : dm{d} {}
    void setRoot(Instruction* r);
    bool HasContainerColumns(const wxDataViewItem& item) const override;
};
