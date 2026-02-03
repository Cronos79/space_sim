#pragma once

namespace commands { class Router; }
namespace sim { class GameClock; }

namespace sim_cmd {
void register_time_utils(commands::Router& r, const sim::GameClock& clock);
}
