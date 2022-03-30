#include "geometry.h"

#include <spdlog/spdlog.h>

#include <cereal/archives/json.hpp>
#include <cereal/archives/xml.hpp>
#include <cereal/cereal.hpp>
#include <cereal/types/unordered_map.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/types/vector.hpp>
#include <fstream>

bool CasVisManager::loadComponentLibrary(const std::string& fname) {
    std::ifstream file;
    file.open(fname);
    if (!file.is_open()) {
        spdlog::error("Failed to open file {}", fname);
        return false;
    }
    std::unordered_map<CasSlot, Drawable> temp;
    cereal::XMLInputArchive archive(file);
    archive(temp);
    component_library.insert(temp.begin(), temp.end());
    comp_file_name = fname;
    spdlog::debug("Successfully {} components from file {}",
                  std::size(component_library), fname);
    return true;
}

bool CasVisManager::loadLocationLibrary(const std::string& fname) {
    std::ifstream file;
    file.open(fname);
    if (!file.is_open()) {
        spdlog::error("Failed to open file {}", fname);
        return false;
    }
    cereal::XMLInputArchive archive(file);

    archive(location_library);
    loc_file_name = fname;
    spdlog::debug("Successfully {} locations from file {}",
                  std::size(location_library), fname);
    return true;
}

bool CasVisManager::constructVisMap(Configuration* c) {
    spdlog::debug(
        "Constructing visualization map using location file {} and component "
        "library {}.",
        comp_file_name, loc_file_name);
    for (const auto& pair : location_library) {
        auto found = std::find_if(
            component_library.begin(), component_library.end(),
            [&](auto&& x) { return pair.first.isSameType(x.first); });
        if (found == std::end(component_library)) {
            spdlog::error(
                "Requested shape of component that does not exist for {}",
                pair.first.toString());
            continue;
        };
        //        spdlog::debug("Requested shape of component {} found",
        //                      pair.first.toString());
        auto drawable =
            std::make_unique<DrawableCasElement>(pair.second, found->second);
        for (auto& poly : drawable->drawable.shapes) {
            poly.color.getColorPtrFromName(c);
        }
        drawable->cas_slot = pair.first;
        cmap.insert({pair.first, std::move(drawable)});
    }
    spdlog::debug(
        "Successfully loaded {} in to visualization map, now have {} "
        "components in map",
        std::size(location_library), std::size(cmap));
    return true;
}

DrawableCasElement* CasVisManager::getElement(const CasSlot& c) const {
    try {
        auto& ptr = cmap.at(c);
        spdlog::debug(
            "Succesffully got the drawable associated with the cas elemement "
            "{}.",
            c.toString());
        return ptr.get();
    } catch (std::exception& e) {
        spdlog::debug(
            "Failed to find cas slot {} in map with hash {}, despite request.",
            c.toString(), std::hash<CasSlot>{}(c));
    }
    return nullptr;
}

const std::vector<DrawableCasElement*> CasVisManager::getElements(
    const std::vector<CasSlot>& c) const {
    std::vector<DrawableCasElement*> ret;
    for (const auto& p : cmap) {
        auto f = std::find(c.begin(), c.end(), p.first);
        if (f != c.end()) {
            auto& ptr = p.second;
            ret.push_back(ptr.get());
        }
    }
    return ret;
}

const std::vector<DrawableCasElement*> CasVisManager::getAll() const {
    std::vector<DrawableCasElement*> ret;
    for (const auto& p : cmap) {
        auto& ptr = p.second;
        ret.push_back(ptr.get());
    }
    return ret;
}

const std::vector<DrawableCasElement*> CasVisManager::getForCas(
    SlotCassette::PlaneID cas_idx) const {
    // spdlog::debug("Getting components for cassette {}", cas_idx);
    std::vector<DrawableCasElement*> ret;
    spdlog::debug("Getting components for cassette {}", cas_idx);
    for (const auto& p : cmap) {
        if (p.first.getCassette() == cas_idx) {
            spdlog::debug("Found {} == {}", p.first.getCassette(), cas_idx);
            ret.push_back(p.second.get());
        }
    }
    spdlog::debug("Found {} drawable elements for cassette {}", std::size(ret),
                  cas_idx);
    return ret;
}

void CasVisManager::setElementPresent(const CasSlot& c, bool b) {
    cmap.at(c)->setPresent(b);
}

std::vector<CasSlot> CasVisManager::getSlotsForCas(
    SlotCassette::PlaneID cas) const {
    std::vector<CasSlot> ret;
    for (const auto& p : cmap) {
        if (p.first.getCassette() == cas) ret.push_back(p.first);
    }
    spdlog::debug("Found {} slots for cassette {}", std::size(ret), cas);
    return ret;
}
