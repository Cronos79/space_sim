#include <cstdlib>
#include <iostream>
#include <string>

#include "httplib.h"

#include "dev_universe.h"
#include "internal_cmd_api.h"

#include "commands/commands.h"
#include "commands/cmd_universe.h"

static std::string env_str(const char* k, const char* fallback) {
    if (const char* v = std::getenv(k)) return std::string(v);
    return std::string(fallback);
}

int main() {
    std::cout << "sim_server starting...\n";

    const std::string internal_key =
        env_str("SPACE_SIM_INTERNAL_KEY", "dev123");

    // Build dev universe
    universe::Universe u = sim::build_dev_universe();

    // Router + commands
    commands::Router router;
    commands::register_universe_commands(router, u);

    // Internal API server
    httplib::Server server;
    sim::register_internal_cmd_api(server, internal_key, router);

    std::cout << "sim_server listening on 127.0.0.1:8090\n";
    server.listen("127.0.0.1", 8090);
    return 0;
}
