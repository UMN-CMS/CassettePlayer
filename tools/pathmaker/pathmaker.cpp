//#include <spdlog/sinks/basic_file_sink.h>
// #include <spdlog/sinks/rotating_file_sink.h>  // support for rotating file
// logging #include <spdlog/sinks/stdout_color_sinks.h>
#include <wx/app.h>
#include <wx/frame.h>

#include "geometry/geometry.h"
#include "visualization/visualization.h"
#include "visualization/drawables.h"
#include "config/config.h"

class MainFrame : public wxFrame {
    VisualizationCanvas* vc;
    SlotCassette::PlaneID current_cas = {17, 0, 0};
    std::vector<std::unique_ptr<DrawableCasElement>> additions;

   public:
    CasVisManager* cvm;
    MainFrame(CasVisManager*, wxFrame* parent, wxWindowID id = wxID_ANY,
              const wxString& title = wxEmptyString,
              const wxPoint& pos = wxDefaultPosition,
              const wxSize& size = wxSize(1200, 700),
              long style = wxDEFAULT_FRAME_STYLE | wxTAB_TRAVERSAL);
    void initializeGeometry();
};

MainFrame::MainFrame(CasVisManager* _cvm, wxFrame* parent, wxWindowID id,
                     const wxString& title, const wxPoint& pos,
                     const wxSize& size, long style)
    : wxFrame(parent, id, title, pos, size, style) {
    SetSizeHints(wxDefaultSize, wxDefaultSize);
    vc = new VisualizationCanvas(this);
    cvm = _cvm;
    initializeGeometry();

    PointVec pv = {{0, 0}, {100, 100}};
    auto temp = StyledPolygon{pv, 0xFF0000};
    temp.line_width=4;

    additions.emplace_back(new DrawableCasElement(PositionInfo{{0, 0}, 0}, Drawable({temp})));
    std::vector<DrawableCasElement*> to_add;
    for (const auto& ptr : additions) {
        to_add.push_back(ptr.get());
    }
    vc->addElements(to_add);
}

void MainFrame::initializeGeometry() {
    if (cvm != nullptr) {
        spdlog::debug("Initializing geometry for geometry canvas {}",
                      (void*)this);
        //    vc->setAngle(-3.1415/3 * 2);
        vc->mouse_or_center = false;
        auto v = cvm->getForCas(current_cas);
        vc->setElements(v);
    } else {
        spdlog::error(
            "Attempted to initialize geometry for canvas {} with nullptr "
            "DataManager",
            (void*)this);
    }
}

class PathMaker : public wxApp {
   private:
    MainFrame* main_frame;
    CasVisManager cvm;
    Configuration config;

   public:
    bool OnInit() override;
};

bool PathMaker::OnInit() {
    //    spdlog::set_level(spdlog::level::debug);  // Set global log level to
    //    debug
    //  auto console_sink =
    //  std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    //  console_sink->set_level(spdlog::level::debug);

    //  auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
    //      "pathmaker.log", 1024 * 1024 * 5, 2, true);
    //  file_sink->set_level(spdlog::level::trace);

    //  auto logger = std::shared_ptr<spdlog::logger>(
    //      new spdlog::logger("logger", {console_sink, file_sink}));

    //  logger->set_level(spdlog::level::trace);
    //  spdlog::register_logger(logger);
    //  spdlog::set_default_logger(logger);
    cvm.loadComponentLibrary("shapes.xml");
    cvm.loadLocationLibrary("points.xml");
    cvm.constructVisMap(&config);

    main_frame = new MainFrame(&cvm, nullptr);
    main_frame->Show();

    return true;
}

IMPLEMENT_APP(PathMaker)