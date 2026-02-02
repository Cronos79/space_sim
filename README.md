# Space Sim

**Space Sim** is a persistent, text-based space simulation game where the universe evolves independently of player presence.

The universe is the protagonist.

AI factions wage war, expand, trade, collapse, and reshape space whether players are online or not.  
Players begin as small actors with a single space station, slowly carving out influence in a vast, hostile, living universe.

This project prioritizes **simulation, emergent story, and long-term consequence** over scripted content.

---

## Core Design Philosophy

- The universe exists and changes without players
- AI factions drive history
- Power is slow, expensive, and fragile
- Geography matters (distance, gates, chokepoints)
- Loss is permanent
- Story emerges from systems, not quests

Players do not save the universe.  
They survive, adapt, and sometimes reshape small parts of it.

---

## Project Structure

This repository is organized into multiple projects with clear responsibilities:

/core
Shared C++ library

SQLite database access

schema & migrations

authentication

sessions

shared domain logic

/api_server
HTTP API server

login / registration

session handling

player-facing data endpoints

talks to core

/sim_server
Universe simulation server

AI factions

economy

warfare

world ticks

authoritative game state

/web
Web frontend

text-based UI

interacts with api_server

/docs
Design documents

frozen rules and constraints


---

## Current Status

🟢 **Foundation Complete**
- Database schema and migrations
- User registration & login
- Session handling
- API server running
- Core project structure in place

🟡 **Design-Heavy Phase**
- Universe structure defined
- Stargate network rules frozen
- Faction behavior defined
- Station and economy concepts drafted

🔴 **Not Implemented Yet**
- Full universe simulation
- Faction AI behavior
- Trade routes
- Player stations and fleets
- Combat systems

This is intentional.  
Design decisions are being locked down **before** heavy implementation.

---

## Universe Overview

- ~500 solar systems
- Fixed stargate network
- Combination of:
  - core faction systems
  - frontier regions
  - outlaw zones
  - dead/unclaimed systems
- 5 major AI factions
- Numerous minor factions and pirate groups
- Real-time tick-based simulation
- Time is accelerated (the universe moves even when you are offline)

See `/docs` for detailed design documents.

---

## Design Documents

Key documents live in `/docs` and define rules that implementation must follow:

- `stargate_network.md` — fixed universe geography and travel rules
- `universe_factions.md` — AI faction behavior and political dynamics

These documents are treated as **design contracts**.

---

## Tech Stack

- **Language:** C++
- **Database:** SQLite
- **API:** HTTP (cpp-httplib)
- **Serialization:** JSON (nlohmann/json)
- **Frontend:** Web-based text UI
- **Build:** CMake

---

## Philosophy on Scope

This project is intentionally built to grow slowly.

There is no rush to:
- add flashy features
- implement combat early
- over-optimize systems

The goal is a **stable, evolving universe** that can run for years.

---

## Disclaimer

This is a hobby project built for learning, experimentation, and long-term iteration.

Expect:
- refactors
- changing ideas
- unfinished systems

But expect the **core vision** to remain stable.

---

## License

This project is licensed under the **GNU General Public License v3.0**.  
See `LICENSE.md` for details.

Copyright (c) 2026 Matthew "Cronos" B
