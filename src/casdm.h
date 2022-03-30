#pragma once

#include <filesystem>

#include "core/instruction.h"
#include "core/operation.h"
#include "geometry/geometry.h"

class DataManager {
    InstructionManager instruct_mgr;
    OperationRecord op_rec;
    CasVisManager vis_mgr;


    std::filesystem::path operation_file = "operations.xml";
    bool saveOperationToDocument(const std::filesystem::path& path);

   public:
    std::string current_user = "Anonymous";
    SlotCassette::PlaneID current_cas = {33, 0, 0};
    std::filesystem::path getCurrentPath() const { return operation_file; }

    CasVisManager& visMgr() { return vis_mgr; }
    OperationRecord& opRecord() { return op_rec; }
    InstructionManager& insMgr() { return instruct_mgr; }
    std::vector<DrawableCasElement*> getVisForCurrentCas();
    std::vector<CasSlot> getSlotsForCurrentCas();

    bool loadOperationsDocument(const std::filesystem::path& path);
    bool saveOperationDocument();
    bool createNewOperationDocument(const std::filesystem::path& path);
    bool saveTempOperationsRecord();

    bool setInstructionRootByCas(SlotCassette::PlaneID plane);
    bool setInstructionRootByCas();

    bool attemptOperation(Operation o);
    bool attemptCompleteInstruction(
        const std::pair<Instruction*, ExternalComponent>& val);

    void updateAllPresentForCas();
    float getRenderAngleForCas();

    bool isInstructionComplete(Instruction* s);
};
