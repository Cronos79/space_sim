#include "dev_universe.h"

namespace sim {

universe::Universe build_dev_universe() {
    universe::Universe u;

    // systems
    u.add_system({1, "sol"});
    u.add_system({2, "vega"});
    u.add_system({3, "eos"});
    u.add_system({4, "kelnor"});

    // gates (sol->vega->eos->kelnor)
    u.gates().add_gate(1, 2);
    u.gates().add_gate(2, 3);
    u.gates().add_gate(3, 4);

    // a shortcut to make routes interesting (sol->eos)
    u.gates().add_gate(1, 3);

    return u;
}

} // namespace sim
