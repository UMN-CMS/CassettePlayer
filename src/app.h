#pragma once

#include <wx/app.h>

#include "casdm.h"
#include "instruction_frame.h"
#include "config/config.h"

class CassetteApp : public wxApp {
   private:
    InstructionFrame* ins_frame;

    // Must exist for the lifetime of the application
    DataManager data_manager;
    Configuration config;

   public:
    bool OnInit() override;
};

DECLARE_APP(CassetteApp)
