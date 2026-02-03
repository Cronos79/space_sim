#pragma once
#include <cstdint>
#include <string>

namespace time_sim {

// Formats seconds into human-readable duration.
// Examples:
//  65        -> "1m 5s"
//  3600      -> "1h"
//  90061     -> "1d 1h 1m"
//  0         -> "0s"
std::string format_duration(int64_t seconds);

} // namespace time_sim
