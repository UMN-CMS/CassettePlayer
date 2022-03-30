#include "casdm.h"

#include <fstream>

std::vector<DrawableCasElement*> DataManager::getVisForCurrentCas() {
    return visMgr().getForCas(current_cas);
}

bool DataManager::attemptOperation(Operation o) {
    if (opRecord().addOperation(o)) {
        spdlog::debug(
            "Data manager registered operation completed, current operation "
            "record is\n {}",
            opRecord().toString());
        updateAllPresentForCas();
        saveOperationDocument();
        return true;
    } else {
        spdlog::debug("Failed to add operation to record");
        return false;
    }
}

bool DataManager::loadOperationsDocument(const std::filesystem::path& path) {
    auto& opr = opRecord();
    std::ifstream file;
    file.open(path);
    if (!file.is_open()) {
        spdlog::error("Failed to open file {}", path.string());
        return false;
    }
    operation_file = path;
    opr.loadRecord(file);
    return true;
}

bool DataManager::saveOperationToDocument(const std::filesystem::path& path) {
    std::ofstream file;
    file.open(path);
    if (!file.is_open()) {
        spdlog::error("Failed to open file {}", path.string());
        return false;
    }
    auto& opr = opRecord();
    opr.saveRecord(file);
    return true;
}

bool DataManager::saveTempOperationsRecord() {
    return saveOperationToDocument("random.xml");
}

bool DataManager::saveOperationDocument() {
    return saveOperationToDocument(operation_file);
}

bool DataManager::createNewOperationDocument(
    const std::filesystem::path& path) {
    operation_file = path;
    return true;
}

bool DataManager::setInstructionRootByCas(SlotCassette::PlaneID plane) {
    spdlog::debug("Setting instructions to cassette {}", plane);
    return insMgr().setRootByType(CasSlot(SlotCassette(plane)));
}

bool DataManager::setInstructionRootByCas() {
    return setInstructionRootByCas(current_cas);
}

std::vector<CasSlot> DataManager::getSlotsForCurrentCas() {
    spdlog::debug("Getting slots for current cassette");
    return visMgr().getSlotsForCas(current_cas);
}

void DataManager::updateAllPresentForCas() {
    for (const auto& c : getSlotsForCurrentCas()) {
        if (opRecord().hasBeenAssigned(c)) {
            spdlog::debug("Component {} is now set to present", c.toString());
            visMgr().setElementPresent(c, true);
        } else {
            visMgr().setElementPresent(c, false);
        }
    }
}

float DataManager::getRenderAngleForCas() {
    //   float angle = -2 * 3.14 / 6 * (static_cast<double>(current_cas &
    //   0b11));
    float angle = 0;
    return angle;
}

bool DataManager::attemptCompleteInstruction(
    const std::pair<Instruction*, ExternalComponent>& val) {
    if (!val.first) {
        spdlog::error("Op panel has no current instruction");
        return false;
    }
    if (val.first->op_type == OpType::EMPTY) {
        spdlog::error("Attempting to complete an empty instruction.");
        return false;
    }
    if (!val.first->cas_slot) {
        spdlog::debug(
            "Attempting to complete instruction with no associated cas slot: "
            "{}",
            val.first->toString());
        return false;
    }
    Operation newop(val.first->cas_slot.value(), val.second);
    newop.user = current_user;
    return attemptOperation(newop);
}

bool DataManager::isInstructionComplete(Instruction* s) {
    return insMgr().isComplete(s, opRecord());
}
