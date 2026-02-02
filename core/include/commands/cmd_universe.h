#pragma once
#include "commands/commands.h"
#include "universe/universe.h"

namespace commands {

// Registers: system, gates, route, nearby
void register_universe_commands(Router& r, const universe::Universe& u);

} // namespace commands
