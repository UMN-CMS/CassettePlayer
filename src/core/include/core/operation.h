#pragma once

#include <cereal/cereal.hpp>
#include <cereal/types/vector.hpp>
#include <chrono>
#include <iostream>
#include <optional>
#include <unordered_map>

#include "slots.h"

enum OpType {
    EMPTY = 0,
    INSERT_COMPONENT = 1,
    REMOVE_COMPONENT = 2,
    TEST_COMPONENT = 3
};

std::string opTypeToString(OpType t);
OpType stringToOpType(const std::string& t);

namespace cereal {
template <class Archive>
inline std::string save_minimal(Archive const&, OpType const& t) {
    return opTypeToString(t);
}

template <class Archive>
inline void load_minimal(Archive const&, OpType& t, std::string const& value) {
    t = stringToOpType(value);
}
}  // namespace cereal

class Operation {
   public:
    CasSlot cas_slot;
    ExternalComponent ext_comp;

    OpType op_type = OpType::INSERT_COMPONENT;

    std::string additional = "";

    std::string user = "Unknown Operator";

    Operation() = default;
    Operation(const CasSlot& c, const ExternalComponent& e)
        : cas_slot{c}, ext_comp{e} {}

    template <class Archive>
    void serialize(Archive& archive) {
      archive( cas_slot,
                cereal::make_nvp("ExternalComp", ext_comp),
                cereal::make_nvp("OpType", op_type),
                cereal::make_nvp("User", user));
    }
};

class OperationRecord {
    std::vector<Operation> history;
    std::unordered_map<CasSlot, ExternalComponent> current_state;

    std::string assembly_id;

    bool is_modified = false;

    void recomputeCurrentState();
    void performOperationOnState(const Operation& o);

   public:
    std::string getIthOp(std::size_t idx) const;
    std::size_t getNumOperations() const { return std::size(history); }

    std::string toString() const;
    bool isModified() const { return is_modified; }
    const std::vector<Operation>& getRecord() { return history; }

    // Is there an external component in the slot
    bool hasBeenAssigned(const CasSlot& c) const;

    // What is the external component in the slot
    std::optional<ExternalComponent> getCasSlotIdentifier(
        const CasSlot& c) const;

    // Has an external component been used in some slot
    bool isExternalComponentUsed(const ExternalComponent& e) const;

    // Return the location of the cassette component where
    std::optional<CasSlot> getWhereExternalComponentUsed(
        const ExternalComponent& e) const;

    // Add the given operation to the operations history, then save the
    // document.
    bool addOperation(Operation o);

    std::istream& loadRecord(std::istream& stream);
    std::ostream& saveRecord(std::ostream& stream);
};
