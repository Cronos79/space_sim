#pragma once
#include <cstdint>
#include <string>

namespace universe {

using SystemId = int32_t;

enum class SystemType : uint8_t { Core, Frontier, Dead };
enum class SecurityLevel : uint8_t { High, Medium, Low, None };

struct SolarSystem {
    SystemId id{};
    std::string name;

    SystemType type{SystemType::Frontier};
    SecurityLevel security{SecurityLevel::Low};

    int32_t owner_faction_id{0}; // 0 = unclaimed
};

} // namespace universe
