#pragma once

#include <iostream>
#include <unordered_map>

#include "core/slots.h"
#include "visualization/drawables.h"
#include "vmath/vmath.h"

class Configuration;

class CasVisManager {
   private:
    bool is_fully_loaded = false;
    std::unordered_map<CasSlot, std::unique_ptr<DrawableCasElement>> cmap;

    std::unordered_map<CasSlot, Drawable> component_library;
    std::unordered_map<CasSlot, PositionInfo> location_library;

    std::string comp_file_name;
    std::string loc_file_name;

   public:
    bool loadComponentLibrary(const std::string& fname);
    bool loadLocationLibrary(const std::string& fname);
    bool constructVisMap(Configuration* c);

    bool isLoaded() const { return is_fully_loaded; }
    //    bool loadFromFile(const std::string& filename);

    std::string assoc_geo_file = "";

    DrawableCasElement* getElement(const CasSlot& c) const;

    void setElementPresent(const CasSlot& c, bool present = false);

    const std::vector<DrawableCasElement*> getElements(
        const std::vector<CasSlot>& c) const;
    const std::vector<DrawableCasElement*> getAll() const;
    const std::vector<DrawableCasElement*> getForCas(SlotCassette::PlaneID) const;
    std::vector<CasSlot> getSlotsForCas(SlotCassette::PlaneID) const;
};
