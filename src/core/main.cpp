#include <iostream>
#include <sstream>

#include "fmt/core.h"
#include "instruction.h"
#include "operation.h"
#include "slots.h"
#include "spdlog/spdlog.h"

int main() {
    spdlog::set_level(spdlog::level::debug);  // Set global log level to debug
    spdlog::set_pattern("[%H:%M:%S %z] [%s:%#] %v");
    InstructionManager insmgr;
    OperationDocument opdoc;
    Operation o(SlotModule(1, 2, 23),
                ExternalComponent(CompType::Module, "231234"));
    opdoc.addOperation(o);
    //opdoc.SaveObject(std::cout);

    auto i1 = std::make_shared<Instruction>(
        SlotModule(1, 2, 23), OpType::INSERT_COMPONENT, "Test INstruction");
    auto i2 = std::make_shared<Instruction>(
        SlotModule(4, 5, 6), OpType::INSERT_COMPONENT, "New instruction");
    auto i3 = std::make_shared<Instruction>(
        SlotEngine(SlotModule(30, 5, 6), SlotModule(20, 21, 23)),
        OpType::INSERT_COMPONENT, "Other instruction");
    i1->children.push_back(i2);
    i1->children.push_back(i3);
    i2->children.push_back(i3);
    insmgr.instructions.push_back(i1);
    insmgr.instructions.push_back(i2);
    insmgr.instructions.push_back(i3);
    std::stringstream ss;
    std::stringstream iss;
    std::string s;
    //insmgr.saveInstructions(ss);
    //insmgr.loadInstructions(ss);
    insmgr.saveInstructions(ss);
    std::cout << ss.str();

    return 0;
}
