#pragma once
#include "core/instruction.h"
#include <wx/dataview.h>

inline Instruction* toInstruction(const wxDataViewItem& item) {
    return reinterpret_cast<Instruction*>(item.GetID());
}

inline wxDataViewItem fromInstruction(Instruction* ins) {
    return wxDataViewItem(reinterpret_cast<void*>(ins));
}
