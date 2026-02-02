#include "universe/universe.h"

namespace universe {

bool Universe::add_system(SolarSystem sys) {
    if (systems_.contains(sys.id)) return false;
    systems_.emplace(sys.id, sys);
    gates_.add_node(sys.id);
    return true;
}

std::optional<SolarSystem> Universe::get_system(SystemId id) const {
    auto it = systems_.find(id);
    if (it == systems_.end()) return std::nullopt;
    return it->second;
}

} // namespace universe
