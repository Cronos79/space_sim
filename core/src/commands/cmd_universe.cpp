#include "commands/cmd_universe.h"
#include <sstream>

namespace commands {

static std::string sys_name(const universe::Universe& u, universe::SystemId id) {
    auto s = u.get_system(id);
    return s ? s->name : ("#" + std::to_string(id));
}

void register_universe_commands(Router& r, const universe::Universe& u) {

    r.add("gates", [&u](const Context&, const Command& cmd) -> Result {
        if (cmd.args.size() != 1) {
            return {false, "Usage: gates <system>", "usage", {}};
        }
        auto sid = u.find_system_by_name(cmd.args[0]);
        if (!sid) return {false, "Unknown system: " + cmd.args[0], "unknown_system", {}};

        std::ostringstream out;
        out << "Gates from " << cmd.args[0] << ":\n";
        for (auto n : u.gates().neighbors(*sid)) {
            out << "  - " << sys_name(u, n) << "\n";
        }
        return {true, out.str(), "", {}};
    });

    r.add("route", [&u](const Context&, const Command& cmd) -> Result {
        if (cmd.args.size() != 2) {
            return {false, "Usage: route <from> <to>", "usage", {}};
        }
        auto a = u.find_system_by_name(cmd.args[0]);
        auto b = u.find_system_by_name(cmd.args[1]);
        if (!a) return {false, "Unknown system: " + cmd.args[0], "unknown_system", {}};
        if (!b) return {false, "Unknown system: " + cmd.args[1], "unknown_system", {}};

        auto rr = u.gates().shortest_route(*a, *b);
        if (!rr) return {false, "No route found.", "no_route", {}};

        std::ostringstream out;
        out << "Route: " << cmd.args[0] << " -> " << cmd.args[1] << "\n";
        out << "Jumps: " << rr->jumps << "\n";
        out << "Path:\n";
        for (size_t i = 0; i < rr->path.size(); ++i) {
            out << "  " << (i + 1) << ") " << sys_name(u, rr->path[i]) << "\n";
        }
        return {true, out.str(), "", {}};
    });

    r.add("nearby", [&u](const Context&, const Command& cmd) -> Result {
        if (cmd.args.size() != 2) {
            return {false, "Usage: nearby <system> <N>", "usage", {}};
        }
        auto sid = u.find_system_by_name(cmd.args[0]);
        if (!sid) return {false, "Unknown system: " + cmd.args[0], "unknown_system", {}};

        int n = 0;
        try { n = std::stoi(cmd.args[1]); }
        catch (...) { return {false, "N must be a number.", "bad_number", {}}; }

        auto ids = u.gates().within(*sid, n, false);

        std::ostringstream out;
        out << "Systems within " << n << " jumps of " << cmd.args[0] << ":\n";
        for (auto id : ids) out << "  - " << sys_name(u, id) << "\n";
        return {true, out.str(), "", {}};
    });

    // You can add "system" later once SolarSystem has more fields you care about.
}

} // namespace commands
