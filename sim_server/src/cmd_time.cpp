#include "cmd_time.h"

#include <sstream>

#include "commands/commands.h"   // core command types
#include "game_clock.h"          // sim_server clock
#include "time/game_time.h"      // core formatting

namespace sim_cmd {

void register_time_commands(commands::Router& r, const sim::GameClock& clock) {
    r.add("time", [&clock](const commands::Context&, const commands::Command& cmd) -> commands::Result {
        if (!cmd.args.empty()) {
            return {false, "Usage: time", "usage", {}};
        }

        std::ostringstream out;
        out << "Time: " << clock.now_gst() << "\n";
        out << "GameSeconds: " << clock.now_game_seconds() << "\n";
        out << "Ticks: " << clock.tick_count() << "\n";
        out << "Scale: " << clock.cfg().game_seconds_per_real_second << " game sec / real sec\n";
        out << "TickStep: " << clock.cfg().tick_step_game_seconds << " game sec\n";
        return {true, out.str(), "", {}};
    });
}

} // namespace sim_cmd
