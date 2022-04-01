#include "common.h"

std::vector<Row> extractRows(const std::string& file) {
    rapidcsv::Document doc;
    doc.Load(file, rapidcsv::LabelParams(0, -1),
             rapidcsv::SeparatorParams(' '));
#define GET_COL(name, type) std::vector<type> name = doc.GetColumn<type>(#name)
#define GET_COLF(name, type, func) \
    std::vector<type> name = doc.GetColumn<type>(#name, func)
#define GET_COL_ARR(name, num) name[num] = doc.GetColumn<float>(#name "_" #num)
    std::vector<Row> ret;
    ret.reserve(5000);
    std::array<std::vector<float>, 7> vx, vy;

    GET_COL(nvertices, int);
    GET_COL_ARR(vx, 0);
    GET_COL_ARR(vx, 1);
    GET_COL_ARR(vx, 2);
    GET_COL_ARR(vx, 3);
    GET_COL_ARR(vx, 4);
    GET_COL_ARR(vx, 5);
    GET_COL_ARR(vx, 6);
    GET_COL_ARR(vy, 0);
    GET_COL_ARR(vy, 1);
    GET_COL_ARR(vy, 2);
    GET_COL_ARR(vy, 3);
    GET_COL_ARR(vy, 4);
    GET_COL_ARR(vy, 5);
    GET_COL_ARR(vy, 6);
    GET_COL(MB, int);
    GET_COL(x0, float);
    GET_COL(y0, float);
    GET_COL(phi, float);
    GET_COL(icassette_0, std::size_t);
    GET_COL(icassette_1, std::size_t);
    GET_COL(irot, int);
    GET_COL(mrot, int);
    GET_COLF(isEngine, bool, [](const std::string& in, bool& b) {
        b = (in.find('T') != std::string::npos);
    });
    // GET_COLF(isDepDCDC, bool, [](const std::string& in, bool& b) {
    //     b = (in.find('T') != std::string::npos);
    // });
    GET_COL(u, int);
    GET_COL(v, int);
    GET_COL(plane, std::size_t);
    GET_COL(WagonName, std::string);

    GET_COL(itype, std::string);

    for (std::size_t i = 0; i < std::size(u); ++i) {
        if (i % 1000 == 0) {
            spdlog::debug("Now processing line {}", i);
        }
        Row r;
        r.u = u[i];
        r.v = v[i];
        r.phi = phi[i];
        r.plane = plane[i];
        r.icas_0 = icassette_0[i];
        r.icas_1 = icassette_1[i];
        r.x0 = x0[i];
        r.y0 = y0[i];
        r.MB = MB[i];
        r.is_engine = isEngine[i];
        r.itype = itype[i];
        r.irot = irot[i];
        r.mrot = mrot[i];
        r.wag_name = WagonName[i];
        //   r.dep_dcdc = isDepDCDC[i];

        for (int j = 0; j < nvertices[i]; ++j) {
            r.vertices.push_back(Point({vx[j][i], vy[j][i]}));
        }
        ret.push_back(std::move(r));
    }
    spdlog::debug("Processed {} lines in to rows", std::size(u));
    return ret;
}

void processCassettes(std::unordered_map<CasSlot, PositionInfo>& ret,
                      Channel& c) {
    std::unordered_set<SlotCassette> found_c;
    for (const Row& row : c.rows) {
        SlotCassette newcas(
            SlotCassette::PlaneID{row.plane, row.icas_0, row.icas_1});
        if (found_c.insert(newcas).second) {
            ret.insert({newcas, PositionInfo({0, 0}, 0)});
        }
    }
    std::move(found_c.begin(), found_c.end(), std::back_inserter(c.cass));
}

void processModules(std::unordered_map<CasSlot, PositionInfo>& ret,
                    Channel& c) {
    auto& rows = c.rows;
    auto& found_mods = c.mods;
    int i = 0;
    for (const Row& row : rows) {
        ++i;
        if (i % 1000 == 0) {
            spdlog::debug("Now processing line {}", i);
        }
        int three_idx = getThreeWithAngleLess(row.vertices, 97);
        if (three_idx == -1) {
            spdlog::debug(
                "Did not find three points forming hexagonal side with type {}",
                row.itype);
            continue;
        };
        auto slot = SlotModule(
            row.u, row.v, SlotCassette({row.plane, row.icas_0, row.icas_1}),
            row.itype);
        found_mods.push_back(slot);

        float angle = 0;
        if (row.itype.find("F") != std::string::npos) {
            angle = 2 * 3.1415 * (row.irot - 3) / 6;
        }
        if (row.itype.find("a") != std::string::npos) {
            if (row.itype.find("T") != std::string::npos) {
                angle = 2 * 3.1415 * (row.irot - 2) / 6;
            }
            if (row.itype.find("B") != std::string::npos) {
                angle = 2 * 3.1415 * (row.irot - 1) / 6;
            }
            if (row.itype.find("O") != std::string::npos) {
                angle = 2 * 3.1415 * (row.irot - 2) / 6;
            }
        }
        if (row.itype.find("b") != std::string::npos) {
            if (row.itype.find("e") != std::string::npos) {
                angle = 2 * 3.1415 * (row.irot - 2) / 6;
            } else {
                angle = 2 * 3.1415 * (row.irot + 3) / 6;
            }
        }
        if (row.itype.find("c") != std::string::npos) {
            angle = 2 * 3.1415 * (row.irot - 1) / 6;
        }
        if (row.itype.find("d") != std::string::npos) {
            if (row.itype.find("I") != std::string::npos) {
                angle = 2 * 3.1415 * (row.irot + 2) / 6;
            }
            if (row.itype.find("O") != std::string::npos) {
                angle = 2 * 3.1415 * (row.irot - 1) / 6;
            }
        }

        if (row.itype.find("g") != std::string::npos) {
            angle = 2 * 3.1415 * (row.irot - 3) / 6;
        }
        auto point = PositionInfo({row.x0, row.y0}, angle);
        ret.insert({slot, point});
        if(row.dep_dcdc){
          ret.insert({SlotDepDCDC(slot), point });
        }
    }
}

std::pair<int, int> rotAnalyzer(int rot) {
    int umod = 0, vmod = 0;
    switch (rot) {
        case 0:
        case 1:
            umod = 1;
            break;
        case 5:
        case 2:
            umod = 0;
            break;
        case 4:
        case 3:
            umod = -1;
            break;
    }
    switch (rot) {
        case 1:
            vmod = 1;
            break;
        case 2:
            vmod = 1;
            break;
        case 3:
            vmod = -1;
            break;
        case 5:
            vmod = -1;
            break;
        case 4:
            vmod = -1;
            break;
        case 0:
            vmod = 0;
            break;
    }
    return {umod, vmod};
}

void processEngines(std::unordered_map<CasSlot, PositionInfo>& ret,
                    Channel& c) {
    auto& rows = c.rows;
    auto& found_mods = c.mods;
    for (const Row& row : rows) {
        if (row.is_engine) {
            spdlog::debug(
                "This cas: {}",
                SlotCassette({row.plane, row.icas_0, row.icas_1}).toString());
            auto m = SlotModule(
                row.u, row.v, SlotCassette({row.plane, row.icas_0, row.icas_1}),
                row.itype);
            int rot = row.irot;
            // umod = 0;
            // vmod = 0;
            int umod = 0, vmod = 0;
            std::tie(umod, vmod) = rotAnalyzer(rot);
            auto slot_temp = SlotModule(m.u + umod, m.v + vmod, m.cas);
            auto search =
                std::find_if(found_mods.begin(), found_mods.end(),
                             [&](auto&& x) { return slot_temp.isSamePos(x); });
            if (search == found_mods.end()) {
                spdlog::error(
                    "Attempted to locate engine using module that does not "
                    "exist {} -> {} -> {}",
                    m, row.irot, slot_temp);
                continue;
            } else {
                spdlog::debug("Identified other module\n{} -> {} -> {}", m,
                              row.irot, *search);
            }
            slot_temp.type = search->type;
            Point p1{row.x0, row.y0};
            float perc = 0.5;

            auto diff = -(Point(row.x0, row.y0) - ret[slot_temp].p);
            p1 = (1 - perc) * p1 + perc * ret[slot_temp].p;
            p1 = p1 - 25 * orthDirection(diff);

            auto point = PositionInfo(p1, std::atan(diff.second / diff.first));
            //            spdlog::debug("Creating
            //            module with location
            //            ({},{})", point.x(),
            //                          point.y());
            c.eng.insert({m, SlotEngine(m, slot_temp)});
            c.eng.insert({slot_temp, SlotEngine(m, slot_temp)});
            ret.insert({SlotEngine(m, slot_temp), point});
        }
    }
}

void processWagons(std::unordered_map<CasSlot, PositionInfo>& ret, Channel& c) {
    for (const Row& row : c.rows) {
        if (!row.wag_name.empty()) {
            auto this_mod = SlotModule(
                row.u, row.v, SlotCassette({row.plane, row.icas_0, row.icas_1}),
                row.itype);
            auto& eng = c.eng[this_mod];
            int rotfactor = -1;
            PositionInfo pos_of_mod = ret[eng.left];
            PositionInfo pos_of_eng = ret[eng.right];
            PositionInfo realpos;

            auto diff = rotfactor * (pos_of_eng.p - pos_of_mod.p);

            if (row.wag_name[0] == 'E' && row.wag_name.find("2") != std::string::npos)
                realpos.p = pos_of_eng.p - diff;
            else if (row.wag_name.find("E_")  != std::string::npos)
                realpos = pos_of_eng;
            else if (row.wag_name.find("W_")  != std::string::npos)
                realpos = pos_of_mod;
            else if (row.wag_name[0] == 'W')
                realpos = pos_of_mod;
            else if (row.wag_name[0] == 'E')
                realpos = pos_of_eng;
            auto point =
                PositionInfo(realpos.p, std::atan(diff.second / diff.first));
            ret.insert({SlotWagon(eng, row.wag_name, 0, 0), point});
        }
    }
}

std::unordered_map<CasSlot, PositionInfo> parsePos(
    const std::vector<Row>& rows) {
    std::unordered_map<CasSlot, PositionInfo> ret;
    std::vector<SlotModule> found_mods;
    int i = 0;
    spdlog::debug("Processed {} lines", i);
    Channel c{rows, {}, {}, {}};
    processCassettes(ret, c);
    processModules(ret, c);
    processEngines(ret, c);
    processWagons(ret, c);
    return ret;
}
