#pragma once
#include <string>
#include "httplib.h"
#include "commands/commands.h"

namespace sim {

// Adds internal endpoints to the server.
// Currently: GET /health, POST /cmd
void register_internal_cmd_api(
    httplib::Server& server,
    const std::string& internal_key,
    commands::Router& router
);

} // namespace sim
