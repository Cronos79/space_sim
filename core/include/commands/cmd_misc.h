#pragma once

namespace universe { class Universe; }
namespace commands { class Router; }

namespace commands {

// Adds "random_system" and "random_route" commands.
void register_misc_commands(Router& r, const universe::Universe& u);

} // namespace commands
