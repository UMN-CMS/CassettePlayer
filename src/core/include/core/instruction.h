#pragma once

#include <cereal/cereal.hpp>
#include <chrono>
#include <iostream>
#include <optional>
#include <unordered_map>

#include "operation.h"
#include "slots.h"

struct BadInstructions : public std::exception {
    const char* what() const throw() {
        return "Error parsing instruction file";
    }
};

struct Instruction;

using InsPtr = std::shared_ptr<Instruction>;

struct Instruction {
    std::optional<CasSlot> cas_slot = std::nullopt;
    OpType op_type = OpType::INSERT_COMPONENT;
    std::string description = "";

    std::string toString() const;

    Instruction(std::optional<CasSlot> c, OpType t, std::string desc)
        : cas_slot{c}, op_type{t}, description{desc} {}
    Instruction() = default;
    std::vector<InsPtr> children;
    std::vector<InsPtr> dependencies;
    InsPtr parent;

    template <class Archive>
    void serialize(Archive& archive) {
        archive(cereal::make_nvp("CasSlot", cas_slot),
                cereal::make_nvp("OpType", op_type),
                cereal::make_nvp("Description", description),
                cereal::defer(children), cereal::defer(dependencies),
                cereal::defer(parent));
    }
};

class InstructionManager {
   private:
    template <typename... Args>
    void addIfNotPresent(Args&&... args) {
        (
            [this](auto&& x) {
                if (std::find(this->instructions.begin(),
                              this->instructions.end(),
                              x) == this->instructions.end()) {
                    instructions.push_back(x);
                }
            }(std::forward<Args>(args)),
            ...);
    }

   public:
    std::vector<InsPtr> instructions = {};
    using VertVec = std::vector<InsPtr>;

    Instruction* root = nullptr;

    void addChild(const InsPtr& parent, const InsPtr& child);
    void addDep(const InsPtr& parent, const InsPtr& child);
    void setParentOfTo(const InsPtr& child, const InsPtr& parent);

    std::vector<Instruction*> getTopLevel();
    std::vector<Instruction*> getDirectChildren(Instruction* inst);
    const std::vector<Instruction*> getDirectChildren(Instruction* inst) const;
    std::vector<Instruction*> getAllChildren(Instruction* inst);
    Instruction* getOneParent(Instruction* inst);

    template <typename T>
    bool setRootByType(const T& type) {
        spdlog::debug("Attempting to find root of type {}", type.toString());
        auto found = std::find_if(
            instructions.begin(), instructions.end(),
            [&](auto&& x) { return x->cas_slot->isSameType(type); });
        if (found == instructions.end()) return false;
        root = found->get();
        return true;
    }

    bool isReady(Instruction* inst, const OperationRecord& d) const;
    std::vector<Instruction*> getDirectDepedencies(Instruction* inst);
    // std::vector<Instruction*> getAllDepedencies(Instruction* inst);

    Instruction* getSecParent(Instruction*);
    long getCompletionPercentage(Instruction* p,
                                 const OperationRecord& d) const;
    bool isComplete(Instruction* p, const OperationRecord& d) const;

    Instruction* addInstruction(InsPtr inst) {
        instructions.push_back(std::move(inst));
        return inst.get();
    }

    template <typename T, typename... Args>
    Instruction* addInstruction(OpType optype, std::string desc,
                                Args&&... args) {
        return addInstruction(std::make_unique<Instruction>(std::optional<T>(
                                  std::forward<Args>(args)...)),
                              optype, desc);
    }

    void loadInstructions(std::istream& input);
    void loadInstructions(const std::string& s);
    void saveInstructions(std::ostream& output);

    InstructionManager();

    template <class Archive>
    void serialize(Archive& archive) {
        archive(instructions);
        archive.serializeDeferments();
    }
};
