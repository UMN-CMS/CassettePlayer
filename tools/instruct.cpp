#include <fmt/color.h>
#include <fmt/core.h>
#include <fmt/ranges.h>

// #include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
// #include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>
// #include <iostream>
#include <structopt/app.hpp>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "common.h"
#include "core/instruction.h"
#include "core/slots.h"
#include "rapidcsv.h"
#include "spdlog/spdlog.h"

void makeOneCas(CasSlot main_cas, const std::vector<CasSlot>& slots,
                InstructionManager& insmgr) {
    auto main_root = std::make_shared<Instruction>(
        main_cas, OpType::INSERT_COMPONENT, "MainRoot");
    auto modules_root = std::make_shared<Instruction>(
        std::nullopt, OpType::EMPTY, "Place Modules");
    auto engines_root = std::make_shared<Instruction>(
        std::nullopt, OpType::EMPTY, "Place Engines");
    auto wagons_root = std::make_shared<Instruction>(
        std::nullopt, OpType::EMPTY, "Place Wagons");

    //    insmgr.instructions.push_back(main_root);
    //    insmgr.instructions.push_back(modules_root);
    //    insmgr.instructions.push_back(engines_root);
    //    insmgr.instructions.push_back(wagons_root);
    insmgr.addChild(main_root, modules_root);
    insmgr.addChild(main_root, engines_root);
    insmgr.addChild(main_root, wagons_root);
    int i = 0;
    for (const CasSlot& slot : slots) {
        ++i;
        if (slot.isType<SlotModule>()) {
            auto one_mod = std::make_shared<Instruction>(
                std::nullopt, OpType::EMPTY,
                fmt::format("Mount {}", slot.toString()));
            SlotModule relslot = slot.getValue<SlotModule>();
            auto screw = SlotScrews(relslot, "ScrewType1");
            auto p1 = std::make_shared<Instruction>(
                relslot, OpType::INSERT_COMPONENT, "Insert module");
            auto p2 = std::make_shared<Instruction>(
                screw, OpType::INSERT_COMPONENT, "Random screw");
            SlotDepDCDC depslot = SlotDepDCDC(relslot);
            auto p3 = std::make_shared<Instruction>(
                depslot, OpType::INSERT_COMPONENT, "Insert deportedDCDC");
            insmgr.addChild(one_mod, p1);
            // insmgr.addChild(one_mod, p2);
            // insmgr.addDep(p1, p2);

            insmgr.instructions.push_back(p1);
            insmgr.addChild(one_mod, p3);
            insmgr.addDep(p3,p1);
            // insmgr.instructions.push_back(p2);
            insmgr.instructions.push_back(one_mod);
            insmgr.addChild(modules_root, one_mod);
        }
        if (slot.isType<SlotEngine>()) {
            SlotEngine relslot = slot.getValue<SlotEngine>();
            auto p1 = std::make_shared<Instruction>(
                relslot, OpType::INSERT_COMPONENT, "Insert Engine");
            insmgr.addChild(engines_root, p1);
        }
        if (slot.isType<SlotWagon>()) {
            SlotWagon relslot = slot.getValue<SlotWagon>();
            auto p1 = std::make_shared<Instruction>(
                relslot, OpType::INSERT_COMPONENT, "Insert Wagon");
            insmgr.addChild(wagons_root, p1);
        }

        if (i % 1000 == 0) {
            spdlog::debug("Now processing elements {}", i);
        }
    }
    spdlog::debug("Processed {} elements", std::size(slots));
}
void makeIns(const std::vector<CasSlot>& slots, InstructionManager& insmgr) {
    for (const auto& slot : slots) {
        if (slot.isType<SlotCassette>()) {
            std::vector<CasSlot> slots_for_cas;
            std::copy_if(slots.begin(), slots.end(),
                         std::back_inserter(slots_for_cas),
                         [c = slot.getCassette()](auto&& x) {
                             return x.getCassette() == c;
                         });
            makeOneCas(slot, slots_for_cas, insmgr);
        }
    }
}

struct Options {
    std::optional<std::string> outfile = "out.xml";
    std::string infile;
};

STRUCTOPT(Options, outfile, infile);

int main(int argc, char* argv[]) {
    spdlog::set_level(spdlog::level::debug);
    /*
      std::map<std::string, docopt::value> args =
          docopt::docopt(USAGE, {argv + 1, argv + argc},
                         true,    // show help if requested
                         "0.1");  // version string

      const std::string fname = args["<file>"].asString();
      const std::string out = args["<outfile>"].asString();
    */
    try {
        auto options =
            structopt::app("instruction_maker").parse<Options>(argc, argv);
        std::string fname = options.infile;
        std::string out = options.outfile.value();
        fmt::print("Running on geometry file {}\n", fname);

        auto rows = extractRows(fname);

        auto writeFile = [&](auto&& v) {
            std::ofstream file;
            file.open(out);
            if (!file.is_open()) {
                spdlog::error("Failed to open file {}", fname);
            }
            v.saveInstructions(file);
        };
        InstructionManager insmgr;
        auto res = parsePos(rows);
        std::vector<CasSlot> slots;
        slots.reserve(5000);
        for (const auto& p : res) {
            slots.push_back(p.first);
        }
        makeIns(slots, insmgr);
        writeFile(insmgr);

    } catch (structopt::exception& e) {
        std::cout << e.what() << "\n";
        std::cout << e.help();
    }

    return 0;
}