#include "universe_generator.h"

#include <random>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>

using universe::SystemId;

namespace sim {

    static std::string sys_name(int n) {
        std::ostringstream ss;
        ss << "sys-" << std::setw(4) << std::setfill('0') << n;
        return ss.str();
    }

universe::Universe generate_universe(int system_count, uint32_t seed) {
    universe::Universe u;
    std::mt19937 rng(seed);

    std::uniform_int_distribution<int> pct(0, 99);
    std::uniform_int_distribution<int> pick_id(1, system_count);

    // -----------------------------
    // 1) Create systems
    // -----------------------------
    for (int i = 0; i < system_count; ++i) {
        universe::SolarSystem s;
        s.id = i + 1;
        s.name = sys_name(i + 1);

        int r = pct(rng);        
        if (r < 15) {
            s.type = universe::SystemType::Core;
            s.security = universe::SecurityLevel::High;
        } else if (r < 65) {
            s.type = universe::SystemType::Frontier;
            s.security = universe::SecurityLevel::Medium;
        } else {
            s.type = universe::SystemType::Dead;
            s.security = universe::SecurityLevel::Low;
        }

        s.owner_faction_id = 0;
        u.add_system(std::move(s));
    }

    auto& gates = u.gates();

    // -----------------------------
    // 2) Build a spanning tree (guaranteed connectivity)
    // -----------------------------
    std::vector<SystemId> ids;
    ids.reserve(system_count);
    for (int i = 0; i < system_count; ++i) {
        ids.push_back(i + 1);
    }

    std::shuffle(ids.begin(), ids.end(), rng);

    for (size_t i = 1; i < ids.size(); ++i) {
        SystemId a = pick_id(rng);
        SystemId b = pick_id(rng);
        gates.add_gate(a, b);
    }

    // -----------------------------
    // 3) Add extra random gates (web, shortcuts)
    // -----------------------------
    int extra_edges = system_count / 8; 
    for (int i = 0; i < extra_edges; ++i) {
        SystemId a = (rng() % system_count) + 1;
        SystemId b = (rng() % system_count) + 1;
        if (a != b) {
            gates.add_gate(a, b);
        }
    }

    return u;
}

}
