# Space Sim — Stargate Network Design (v0.1)

## Purpose

The stargate network defines the fixed geography of the universe.
It determines:
- how solar systems connect
- how far regions are from each other
- where chokepoints, borders, and frontier zones emerge

This network is **static** and does not change during the lifetime of the universe.
Politics, factions, trade, and war occur *on top of* this structure.

---

## Scale

- Total solar systems: ~500
- All systems must be reachable from any other system
- No system is guaranteed to be important by default

The universe is designed to feel vast, uneven, and asymmetric.

---

## Core Concepts

### Solar Systems
- Represent nodes in the stargate graph
- Contain planets, moons, asteroid belts, and orbital space
- May be:
  - core faction systems
  - minor faction systems
  - outlaw systems
  - dead / unclaimed systems

---

### Stargates
- Fixed, permanent structures
- Connect exactly two solar systems
- Allow small and medium ships to travel between systems
- Each gate traversal counts as **one jump**

Players and factions cannot create or destroy stargates.

---

### FTL Travel
- Available to large ships
- Does not rely on stargates
- Slower and more expensive than gate travel
- Used for:
  - exploration
  - long-range military movement
  - bypassing gate chokepoints at high cost

FTL does not replace stargates as the primary travel method.

---

## Network Topology Rules

### Connectivity
- The stargate network must be fully connected
- No isolated clusters or unreachable systems
- Long-distance travel should often require multiple jumps

---

### Gate Density
- Average stargates per system: **2–4**
- Minimum: 1 (dead-end systems are allowed)
- Maximum (rare): 5–6 (high-traffic hubs)

Sparse connectivity is intentional.

---

### Backbone Structure
- The network is built on a spanning tree to guarantee connectivity
- This produces:
  - long travel routes
  - natural remoteness
  - frontier regions

---

### Additional Links
- ~15–25% additional gates are added after the backbone
- These create:
  - shortcuts
  - alternate routes
  - strategic chokepoints

Extra links should favor:
- systems that are already well-connected
- systems far apart in the tree (to reduce extreme distances)

---

## Chokepoints and Strategic Value

- Systems with only 1–2 gates are natural bottlenecks
- Systems that connect major regions become:
  - trade hubs
  - military flashpoints
  - pirate hotspots

Control of a gate system is difficult but highly rewarding.

---

## Faction Placement

### Major Factions
- Number of major factions: ~5
- Each major faction controls:
  - 2–3 core systems
- Core systems:
  - are near each other (1–2 jumps apart)
  - have higher gate density
  - are far from other major faction cores (8–15 jumps)

Major faction cores form dense “islands” of power.

---

### Minor Factions
- Control:
  - a single solar system
  - or specific planets/moons within a system
- Often located:
  - near chokepoints
  - in frontier regions
  - adjacent to dead systems

Minor factions may rise, collapse, or be absorbed.

---

### Dead and Outlaw Systems
- No central authority
- Often resource-rich
- High piracy and risk
- Serve as:
  - early player opportunity zones
  - conflict buffers between major powers

These systems are essential for player relevance early on.

---

## Distance and Travel Expectations

- Typical nearby systems: 1–3 jumps
- Regional travel: 4–7 jumps
- Inter-faction core travel: 8–15 jumps
- Extremely remote routes may exceed 15 jumps

Long travel times are intentional and meaningful.

---

## Player Information and UI Expectations

The full stargate map is never shown.

Players interact with the network via:
- local gate listings
- nearby system summaries
- route plotting commands
- jump counts and estimated travel time

Example concepts:
- `gates`
- `route <system>`
- `nearby <n>`
- `distance <system>`

Players build a mental map through experience.

---

## Design Guarantees

- The universe remains navigable without players
- Trade routes emerge naturally from gate geometry
- Borders and conflicts are shaped by geography
- No system is equally important
- Distance always matters

---

## Design Intent Summary

The stargate network is not a convenience system.
It is a strategic constraint that:
- defines power
- creates scarcity
- enables emergent story

The universe does not bend to the player.
The player learns to move within it.
