#include "core/instruction.h"
#include "instructdv.h"
#include "common.h"

static const bool forcetrue=false;

bool InstructionModel::IsContainer(const wxDataViewItem& item) const {
    auto ins = toInstruction(item);
    //    if(!initialized || forcetrue) return false;
    if (ins == nullptr) {
      //    spdlog::trace("Checking  instruction is container {}: true", (void*)toInstruction(item));
        return true;
    }
    //    spdlog::trace("Checking  instruction is container {}: {}", (void*)toInstruction(item), !(dm->insMgr().getDirectChildren(ins).empty()));
    return !(dm->insMgr().getDirectChildren(ins).empty());
}
wxDataViewItem InstructionModel::GetParent(const wxDataViewItem& item) const {
  //    spdlog::trace("Instruction get parent of instruction {}", (void*)toInstruction(item));
    if(!initialized || false){
      //    spdlog::trace("Calling instruction getParent but not initialized");
      return fromInstruction(nullptr);
        }
    auto ins = toInstruction(item);
    if (ins == nullptr || ins == root) {
      //      spdlog::trace("Instruction was nullptr or root of model, returning nullptr");
        return fromInstruction(nullptr);
    }
    Instruction* parent =  dm->insMgr().getOneParent(ins);
    if(parent == dm->insMgr().root){
      //      spdlog::trace("Parent instruction is root of instruction manager, found {} -> {}; returning nullptr instruction instead", (void*)parent, (void*)ins);
      return fromInstruction(nullptr);
    }
    if(parent == root){
      //      spdlog::trace("Parent instruction is root of current model, found {} -> {}; returning nullptr instruction instead", (void*)parent, (void*)ins);
      return fromInstruction(nullptr);
    }
    //    spdlog::trace("Instruction was not nullptr, found {} -> {}", (void*)parent, (void*)ins);
    return fromInstruction(parent);
}

bool InstructionModel::HasContainerColumns(
    const wxDataViewItem& WXUNUSED(item)) const {
    return true;
}

void InstructionModel::GetValue(wxVariant& v, const wxDataViewItem& item,
                                unsigned int col) const {
    auto ins = toInstruction(item);
    if (ins == nullptr) {
        return;
    }
    switch (col) {
        case 0:
            v = wxVariant(ins->toString());
            break;
        case 1:
            v = wxVariant(
                dm->insMgr().getCompletionPercentage(ins, dm->opRecord()));
            break;
        case 2:
            v = wxVariant(dm->insMgr().getCompletionPercentage(
                              ins, dm->opRecord()) == 100);
            break;
        default:
            break;
    }
}
unsigned int InstructionModel::GetChildren(const wxDataViewItem& item,
                                           wxDataViewItemArray& arr) const {

  //if(initialized.count(toInstruction(item) == 0) return 0;
  if(!initialized){
    // spdlog::trace("Calling get children failed bcause not initialized");
    return 0 ;
  }
      

    auto getChildren = [this](Instruction* i) {
        return this->dm->insMgr().getDirectChildren(i);
    };
    auto ins = toInstruction(item);
    //    spdlog::trace("Calling get children of instruction {}", (void*)ins);
    if (ins == nullptr) {
        int i = 0;
        spdlog::trace("Requested item is nullptr");

        if (root == nullptr) {
          //   spdlog::trace("Root of model is also nullptr, so I am getting the top level elements from the isntruction manager");
            auto tl = dm->insMgr().getTopLevel();
            for (Instruction* ptr : tl) {
                ++i;
                //       spdlog::trace("Adding instruction {}", (void*)ptr);
                arr.Add(fromInstruction(ptr));
            }
            //     spdlog::trace("Added {} instructions as children of root", i);
            return i;
        } else {
          // spdlog::trace(
          //              "Data manager has nonnull root {1} and requested children of {2}, so I am calling getChildren from instruction manager for item {1}",
          //    (void*)root, (void*)ins);
            const auto& tl = getChildren(root);
            for (const auto& p : tl) {
                ++i;
                arr.Add(fromInstruction(p));
            }
            return i;
        }
    } else if (getChildren(ins).empty()) {
      //          spdlog::trace("Found no children");
        return 0;
    } else {
      //  spdlog::trace("Now looking for children of non-nullptr instruction");
        auto children = getChildren(ins);
        for (const auto& ptr : children) {
            arr.Add(fromInstruction(ptr));
        }
        // spdlog::trace("Found {} children", std::size(children));
        return std::size(children);
    }
}
unsigned int InstructionModel::GetColumnCount() const { return 3; }
wxString InstructionModel::GetColumnType(unsigned int column) const {
    switch (column) {
        case 0:
            return "string";
            break;
        case 1:
            return "long";
            break;
        case 2:
            return "bool";
            break;
        default:
            return "string";
            break;
    }
}

bool InstructionModel::SetValue(const wxVariant& WXUNUSED(v),
                                const wxDataViewItem& WXUNUSED(item),
                                unsigned int WXUNUSED(col)) {
    return true;
}

void InstructionModel::setRoot(Instruction* r) { root = r; }


void InstructionModel::registerAdditions(Instruction* parent){
    for(Instruction* child: dm->insMgr().getDirectChildren(parent) ){
      //      spdlog::trace("Adding item {} -> {}" ,(void*)parent, (void*)child);
      ItemAdded(fromInstruction(parent),fromInstruction(child));
      registerAdditions(child);
    }
}

void InstructionModel::Initialize(){

#if __APPLE__ && 0
  initialized = true;
#elif __linux__ || __APPLE__
  if(!dm) {
     spdlog::error("Attempting to initialize dataview with nullptr dm");
  }
  //  spdlog::trace("Initializing data model with root of dm={} and root of insmgr ins={}", (void*)root, (void*)dm->insMgr().root);
  initialized = true;
  if(root == nullptr){
  for(Instruction* inst : dm->insMgr().getTopLevel() ) {
    //    spdlog::trace("Adding item {} -> {}" ,(void*)nullptr, (void*)inst);
    ItemAdded(fromInstruction(nullptr),fromInstruction(inst));
    registerAdditions(inst);
  }
  } else {
    for(Instruction* inst : dm->insMgr().getDirectChildren(root)){
      //        spdlog::trace("Adding item {} -> {}" ,(void*)nullptr, (void*)inst);
        ItemAdded(fromInstruction(nullptr),fromInstruction(inst));
        registerAdditions(inst);
  }
  }
#endif
}
