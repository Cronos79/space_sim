#include "cmd_time_utils.h"

#include <sstream>

#include "commands/commands.h"
#include "game_clock.h"
#include "time/game_time.h"
#include "time/duration.h"

namespace sim_cmd {

static bool parse_seconds(const std::string& s, int64_t& out) {
    try {
        out = std::stoll(s);
        return out >= 0;
    } catch (...) {
        return false;
    }
}

void register_time_utils(commands::Router& r, const sim::GameClock& clock) {

    // duration <seconds>
    r.add("duration", [](const commands::Context&, const commands::Command& cmd) -> commands::Result {
        if (cmd.args.size() != 1) {
            return {false, "Usage: duration <seconds>", "usage", {}};
        }

        int64_t secs = 0;
        if (!parse_seconds(cmd.args[0], secs)) {
            return {false, "Invalid seconds value.", "bad_input", {}};
        }

        std::ostringstream out;
        out << "Duration: " << time_sim::format_duration(secs) << "\n";
        out << "Seconds: " << secs;
        return {true, out.str(), "", {}};
    });

    // when <seconds>
    r.add("when", [&clock](const commands::Context&, const commands::Command& cmd) -> commands::Result {
        if (cmd.args.size() != 1) {
            return {false, "Usage: when <seconds>", "usage", {}};
        }

        int64_t secs = 0;
        if (!parse_seconds(cmd.args[0], secs)) {
            return {false, "Invalid seconds value.", "bad_input", {}};
        }

        int64_t future = clock.now_game_seconds() + secs;

        std::ostringstream out;
        out << "Now:  " << clock.now_gst() << "\n";
        out << "Then: " << time_sim::format_gst_datetime(clock.cfg(), future) << "\n";
        out << "In:   " << time_sim::format_duration(secs);

        return {true, out.str(), "", {}};
    });
}

} // namespace sim_cmd
