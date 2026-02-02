#pragma once

#include <cstdint>
#include "universe/universe.h"

namespace sim {

// Generates a connected universe with N systems and a gate network.
// Deterministic for a given seed.
universe::Universe generate_universe(int system_count, uint32_t seed);

}
