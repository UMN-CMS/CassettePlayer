#include "app.h"

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include "spdlog/sinks/rotating_file_sink.h" 
#include <spdlog/spdlog.h>

#include "instruction_frame.h"

IMPLEMENT_APP(CassetteApp)

bool CassetteApp::OnInit() {
    //    spdlog::set_level(spdlog::level::debug);  // Set global log level to
    //    debug
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_level(spdlog::level::debug);

    auto file_sink =
      std::make_shared<spdlog::sinks::rotating_file_sink_mt>("log.txt", 1024*1024*5, 2, true );
    file_sink->set_level(spdlog::level::trace);

    auto logger = std::shared_ptr<spdlog::logger>(
        new spdlog::logger("logger", {console_sink, file_sink}));

    logger->set_level(spdlog::level::trace);
    spdlog::register_logger(logger);
    spdlog::set_default_logger(logger);

    data_manager.visMgr().loadComponentLibrary("shapes.xml");
    data_manager.visMgr().loadLocationLibrary("points.xml");
    data_manager.visMgr().constructVisMap(&config);

    data_manager.insMgr().loadInstructions("ins.xml");

    data_manager.setInstructionRootByCas();
    if (std::filesystem::exists(data_manager.getCurrentPath())) {
        data_manager.loadOperationsDocument(data_manager.getCurrentPath());
    } else {
        data_manager.createNewOperationDocument(data_manager.getCurrentPath());
    }

    ins_frame = new InstructionFrame(&data_manager, nullptr, wxID_ANY);
    ins_frame->createVisualizationFrame();

    ins_frame->Show(true);
    return true;
}
