#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>

#include "httplib.h"

#include "game_clock.h"
#include "cmd_time.h"
#include "cmd_time_utils.h"

#include "internal_cmd_api.h"
#include "universe_generator.h"

#include "commands/commands.h"
#include "commands/cmd_universe.h"
#include "commands/cmd_misc.h"

#ifndef SPACE_SIM_INTERNAL_KEY_DEFAULT
#define SPACE_SIM_INTERNAL_KEY_DEFAULT "dev123"
#endif

static std::string env_str(const char* k, const char* fallback) {
    if (const char* v = std::getenv(k)) return std::string(v);
    return std::string(fallback);
}

int main() {
    std::cout << "sim_server starting...\n";

    const std::string internal_key =
        env_str("SPACE_SIM_INTERNAL_KEY", SPACE_SIM_INTERNAL_KEY_DEFAULT);

    // Game time config
    time_sim::GameTimeConfig tcfg;
    tcfg.start_year = 2350;
    tcfg.start_month = 1;
    tcfg.start_day = 1;
    tcfg.game_seconds_per_real_second = 36.0;
    tcfg.tick_step_game_seconds = 600;

    sim::GameClock clock(tcfg);

    // Universe
    universe::Universe u = sim::generate_universe(500, 1337);

    // Commands
    commands::Router router;
    commands::register_universe_commands(router, u);
    commands::register_misc_commands(router, u);
    sim_cmd::register_time_commands(router, clock);
    sim_cmd::register_time_utils(router, clock);

    // Tick thread (dev-simple; we’ll add clean shutdown later)
    std::thread([&] {
        while (true) {
            clock.update();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }).detach();

    // Internal API server
    httplib::Server server;
    sim::register_internal_cmd_api(server, internal_key, router);

    std::cout << "sim_server listening on 127.0.0.1:8090\n";
    server.listen("127.0.0.1", 8090);

    return 0;
}
