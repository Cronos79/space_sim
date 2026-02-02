#include "commands/cmd_misc.h"

#include <random>
#include <sstream>

#include "commands/commands.h"
#include "universe/universe.h"

namespace commands {

void register_misc_commands(Router& r, const universe::Universe& u) {
    // random_system
    // Usage: random_system
    r.add("random_system", [&u](const Context&, const Command& cmd) -> Result {
        if (!cmd.args.empty()) {
            return {false, "Usage: random_system", "usage", {}};
        }

        const auto& systems = u.systems();
        if (systems.empty()) {
            return {false, "Universe has no systems.", "empty_universe", {}};
        }

        // Pick a random element from unordered_map
        static thread_local std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, systems.size() - 1);

        size_t idx = dist(rng);
        auto it = systems.begin();
        std::advance(it, static_cast<long>(idx));

        std::ostringstream out;
        out << "Random system: " << it->second.name << " (#" << it->second.id << ")\n";
        out << "Tip: system " << it->second.name << "\n";
        return {true, out.str(), "", {}};
    });

    // random_route
    // Usage: random_route <count>
    // Example: random_route 5
    r.add("random_route", [&u](const Context&, const Command& cmd) -> Result {
        if (cmd.args.size() != 1) {
            return {false, "Usage: random_route <count>", "usage", {}};
        }

        int count = 0;
        try {
            count = std::stoi(cmd.args[0]);
        } catch (...) {
            return {false, "count must be a number.", "bad_count", {}};
        }

        if (count <= 0) return {false, "count must be >= 1.", "bad_count", {}};
        if (count > 20) count = 20; // keep output reasonable

        const auto& systems = u.systems();
        if (systems.size() < 2) {
            return {false, "Need at least 2 systems.", "empty_universe", {}};
        }

        static thread_local std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<size_t> dist(0, systems.size() - 1);

        auto pick_name = [&]() -> std::string {
            size_t idx = dist(rng);
            auto it = systems.begin();
            std::advance(it, static_cast<long>(idx));
            return it->second.name;
        };

        std::ostringstream out;
        out << "Random routes:\n";

        int made = 0;
        int tries = 0;
        while (made < count && tries < count * 10) {
            ++tries;
            std::string a = pick_name();
            std::string b = pick_name();
            if (a == b) continue;

            auto ida = u.find_system_by_name(a);
            auto idb = u.find_system_by_name(b);
            if (!ida || !idb) continue;

            auto rr = u.gates().shortest_route(*ida, *idb);
            if (!rr) continue;

            out << "  - " << a << " -> " << b << " : " << rr->jumps << " jumps\n";
            ++made;
        }

        if (made == 0) {
            return {false, "Could not generate routes (unexpected).", "internal", {}};
        }

        return {true, out.str(), "", {}};
    });
}

} // namespace commands
