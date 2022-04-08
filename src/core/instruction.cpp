#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE

#include "instruction.h"

#include <cereal/archives/xml.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/optional.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/unordered_set.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>
#include <numeric>

#include "fmt/core.h"
#include "spdlog/spdlog.h"

InstructionManager::InstructionManager() { instructions.reserve(2000); }

void InstructionManager::loadInstructions(const std::string& fname) {
    std::ifstream file;
    file.open(fname);
    if (!file.is_open()) {
        spdlog::error("Failed to open file {}", fname);
    }
    loadInstructions(file);
}

void InstructionManager::loadInstructions(std::istream& input) {
    cereal::XMLInputArchive archive(input);
    serialize(archive);
}

void InstructionManager::saveInstructions(std::ostream& output) {
    cereal::XMLOutputArchive archive(
        output,
        cereal::XMLOutputArchive::Options().indent(true).sizeAttributes(false));
    serialize(archive);
}

std::vector<Instruction*> InstructionManager::getDirectChildren(
    Instruction* inst) {
    std::vector<Instruction*> ret;
    for (auto child : inst->children) {
        ret.push_back(child.get());
    }
    return ret;
}
const std::vector<Instruction*> InstructionManager::getDirectChildren(
    Instruction* inst) const {
    std::vector<Instruction*> ret;
    for (auto child : inst->children) {
        ret.push_back(child.get());
    }
    return ret;
}

std::vector<Instruction*> InstructionManager::getAllChildren(Instruction* inst) {
    std::vector<Instruction*> ret;
    for (const auto& child : getDirectChildren(inst)) {
        ret.push_back(child);
        auto more = getAllChildren(child);
        std::move(more.begin(), more.end(), std::back_inserter(ret));
        if (std::size(ret) > 1000) {
            return ret;
        }
    }
    return ret;
}

Instruction* InstructionManager::getOneParent(Instruction* inst) {
    if (inst == nullptr) {
        spdlog::error("Attempting to get parent of nullptr instruction");
        return nullptr;
    }
    return inst->parent.get();
}

void InstructionManager::addChild(const InsPtr& p, const InsPtr& c) {
    addIfNotPresent(p, c);
    p->children.push_back(c);
    c->parent = p;
}

long InstructionManager::getCompletionPercentage(
    Instruction* inst, const OperationRecord& d) const {
    if (inst->op_type != OpType::EMPTY && inst->cas_slot) {
        return (d.hasBeenAssigned(inst->cas_slot.value())) ? 100 : 0;
    } else {
        auto children = getDirectChildren(inst);
        if (children.empty()) return 100;
        return std::accumulate(children.begin(), children.end(), 0,
                               [this, d](auto&& x, auto&& y) {
                                   return x +
                                          this->getCompletionPercentage(y, d);
                               }) /
               std::size(children);
    }
}

std::string Instruction::toString() const {
    static const std::unordered_map<OpType, std::string> op_name_map{
        {OpType::INSERT_COMPONENT, "Insert component"},
        {OpType::REMOVE_COMPONENT, "Remove component"},
    };
    if(description.empty()){
        return fmt::format("{} {}", op_name_map.at(op_type),
                           (cas_slot) ? cas_slot->toString() : std::string(""));
    } else {
      return fmt::format(description,(fmt::arg("slot",
                                               (cas_slot) ? cas_slot->toString() : std::string(""))));
    }
}

std::vector<Instruction*> InstructionManager::getTopLevel() {
    if (root == nullptr) {
        std::vector<Instruction*> ret;
        for (const auto& instruction : instructions) {
            if (instruction->parent == nullptr) {
                ret.push_back(instruction.get());
            }
        }
        return ret;
    } else {
        return getDirectChildren(root);
    }
}

bool InstructionManager::isComplete(Instruction* inst,
                                    const OperationRecord& o) const {
    return getCompletionPercentage(inst, o) == 100;
}

bool InstructionManager::isReady(Instruction* inst,
                                 const OperationRecord& d) const {
    return std::all_of(
        inst->dependencies.begin(), inst->dependencies.end(),
        [d, this](auto&& x) { return this->isComplete(x.get(), d); });
}
std::vector<Instruction*> InstructionManager::getDirectDepedencies(
    Instruction* inst) {
    std::vector<Instruction*> ret;
    for (const auto& ptr : inst->dependencies) {
        ret.push_back(ptr.get());
    }
    return ret;
}

void InstructionManager::addDep(const InsPtr& parent, const InsPtr& child) {
    parent->dependencies.push_back(child);
}
