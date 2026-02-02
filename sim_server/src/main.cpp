#include <cstdlib>
#include <iostream>
#include <string>

#include "httplib.h"

#include "dev_universe.h"
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

    universe::Universe u = sim::generate_universe(500, 1337);

    // Router + commands
    commands::Router router;
    commands::register_universe_commands(router, u);
    commands::register_misc_commands(router, u);


    // Internal API server
    httplib::Server server;
    sim::register_internal_cmd_api(server, internal_key, router);

    std::cout << "sim_server listening on 127.0.0.1:8090\n";
    server.listen("127.0.0.1", 8090);
    return 0;
}
