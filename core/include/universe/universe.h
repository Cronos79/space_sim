#pragma once
#include <unordered_map>
#include <optional>
#include "universe/solar_system.h"
#include "universe/gate_network.h"

namespace universe {

class Universe {
public:
    bool add_system(SolarSystem sys);
    std::optional<SolarSystem> get_system(SystemId id) const;

    const std::unordered_map<SystemId, SolarSystem>& systems() const { return systems_; }
    GateNetwork& gates() { return gates_; }
    const GateNetwork& gates() const { return gates_; }
    std::optional<SystemId> find_system_by_name(const std::string& name) const;

private:
    std::unordered_map<SystemId, SolarSystem> systems_;
    GateNetwork gates_;
};

} // namespace universe
