#include "operation.h"

#include <cereal/archives/xml.hpp>

#include "fmt/core.h"
#include "spdlog/spdlog.h"

std::string opTypeToString(OpType t) {
    static const std::unordered_map<OpType, std::string> map{
        {OpType::EMPTY, "Empty"},
        {OpType::INSERT_COMPONENT, "InsertComponent"},
        {OpType::REMOVE_COMPONENT, "RemoveComponent"},
        {OpType::TEST_COMPONENT, "TestComponent"},
    };

    return map.at(t);
}
OpType stringToOpType(const std::string& t) {
    static const std::unordered_map<std::string, OpType> map{
        {"Empty", OpType::EMPTY},
        {"InsertComponent", OpType::INSERT_COMPONENT},
        {"RemoveComponent", OpType::REMOVE_COMPONENT},
        {"TestComponent", OpType::TEST_COMPONENT},
    };
    return map.at(t);
}

std::string opTypeToStringUser(OpType t) {
    static const std::unordered_map<OpType, std::string> map{
        {OpType::EMPTY, "Empty"},
        {OpType::INSERT_COMPONENT, "placed component"},
        {OpType::REMOVE_COMPONENT, "removed component"},
        {OpType::TEST_COMPONENT, "tested component"},
    };

    return map.at(t);
}
std::string gulp(std::istream& in) {
    std::string ret;
    char buffer[4096];
    while (in.read(buffer, sizeof(buffer))) ret.append(buffer, sizeof(buffer));
    ret.append(buffer, in.gcount());
    return ret;
}

std::ostream& OperationRecord::saveRecord(std::ostream& stream) {
    spdlog::debug("Saving File");
    cereal::XMLOutputArchive output{stream};
    output(history);
    return stream;
}

std::istream& OperationRecord::loadRecord(std::istream& stream) {
    spdlog::debug("Loading File");
    cereal::XMLInputArchive input{stream};
    input(history);
    recomputeCurrentState();
    return stream;
}

void OperationRecord::performOperationOnState(const Operation& o) {
    switch (o.op_type) {
        case OpType::INSERT_COMPONENT:
            spdlog::debug("Performing operation INSERT_COMPONENT ");
            current_state.insert({o.cas_slot, o.ext_comp});
            break;
        case OpType::REMOVE_COMPONENT:
            spdlog::debug("Performing operation REMOVE_COMPONENT ");
            current_state.erase(o.cas_slot);
            break;
        case OpType::TEST_COMPONENT:
            spdlog::debug("Performing operation TEST_COMPONENT ");
            break;
        case OpType::EMPTY:
            break;
    }
}

void OperationRecord::recomputeCurrentState() {
    current_state.clear();
    current_state.reserve(std::size(history));
    for (const auto& operation : history) {
        performOperationOnState(operation);
    }
}

std::optional<ExternalComponent> OperationRecord::getCasSlotIdentifier(
    const CasSlot& c) const {
    auto search = current_state.find(c);
    if (search != std::end(current_state))
        return search->second;
    else
        return {};
}
bool OperationRecord::hasBeenAssigned(const CasSlot& c) const {
    auto search = current_state.find(c);
    return search != std::end(current_state);
}

bool OperationRecord::addOperation(Operation o) {
    history.push_back(o);
    performOperationOnState(o);
    return true;
}

std::string OperationRecord::toString() const {
    std::string ret = "Current History";
    int i = 0;
    for (const auto& op : history) {
        ret +=
            fmt::format("OPERATION {}:{} {} {} \n", i, op.cas_slot.toString(),
                        opTypeToString(op.op_type), op.ext_comp.toString());
        ++i;
    }
    ret += " Resulting state\n";
    for (const auto& pair : current_state) {
        ret += fmt::format("{} -> {}\n", pair.first.toString(),
                           pair.second.toString());
    }
    return ret;
}

std::string OperationRecord::getIthOp(std::size_t idx) const {
    const Operation& op = history.at(idx);
    return fmt::format("{} {} {} \n", op.user, opTypeToStringUser(op.op_type),
                       op.cas_slot.toString() );
}
