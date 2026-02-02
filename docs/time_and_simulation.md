# Space Sim — Time and Simulation Design (v0.1)

## Purpose

This document defines how time flows in the universe and how simulation ticks affect:
- factions
- wars
- economy and industry
- player absence
- long-term history

Time is not a convenience system.
It is the engine that makes the universe feel alive.

---

## Core Principle

The universe advances **with or without players**.

- No pause
- No catch-up
- No protection from absence

Players return to a universe that has changed.

---

## Time Scale

The universe operates on a compressed real-time scale.

### Conversion
- **1 real hour ≈ 1.5 in-game days**
- **1 real day ≈ ~1 in-game month**

This compression enables:
- long-term wars within reasonable real time
- economic cycles that matter
- meaningful absence without instant irrelevance

---

## Simulation Ticks

### Tick Definition
A **tick** is the smallest unit of simulation time.

- Ticks occur at fixed real-time intervals
- Each tick advances the state of the universe

Ticks drive:
- faction decisions
- economic production
- ship movement
- construction progress
- combat resolution
- decay and maintenance

---

### Tick Granularity

Not all systems update at the same frequency.

Recommended conceptual layers:

- **Fast ticks** (seconds)
  - ship movement
  - local combat resolution
  - piracy encounters

- **Medium ticks** (minutes)
  - production
  - mining output
  - station activity
  - trade flow

- **Slow ticks** (hours)
  - faction strategy shifts
  - diplomacy changes
  - war escalation or de-escalation
  - population and stability changes

This prevents unnecessary micromanagement and performance pressure.

---

## Player Absence

Absence is expected and supported.

When a player is offline:
- stations continue operating
- ships continue assigned tasks
- trade routes continue
- factions continue to act

Players do **not** miss “clicks.”
They miss **opportunities and outcomes**.

Returning players should see:
- completed projects
- losses incurred
- changes in local politics
- new threats or opportunities

---

## Construction and Industry Over Time

### Construction
- All construction takes real time
- Larger projects take days or weeks of real time
- Construction can be:
  - paused
  - slowed by shortages
  - disrupted by attacks

There are no instant builds.

---

### Industry
- Mining produces resources per tick
- Refining and manufacturing consume time and inputs
- Logistics delays matter

Industrial power scales with:
- infrastructure
- logistics
- time investment

---

## Trade Over Time

Trade routes:
- generate value continuously
- are exposed to risk continuously
- can be disrupted mid-operation

Trade profit is not a button press.
It is the result of sustained, protected activity over time.

---

## Warfare and Time

### War Pace
- Wars are slow and strategic
- Territory does not flip instantly
- Core systems require prolonged effort to threaten

### Attrition
- Wars drain resources over time
- Logistics failures weaken fleets
- Long wars destabilize factions internally

### Resolution
Wars may:
- stall
- end in stalemates
- reshape borders gradually
- collapse factions after prolonged failure

Instant victories are rare.

---

## Maintenance and Decay

Time enforces cost.

Over time:
- stations require upkeep
- ships require maintenance
- neglected infrastructure degrades
- overstretched factions weaken

Power that is not maintained will decline.

---

## Historical Continuity

The universe remembers.

- destroyed stations remain destroyed
- lost wars have lasting consequences
- faction collapse leaves power vacuums
- regions develop reputations over time

History is not reset or forgotten.

---

## Player Time Investment Philosophy

The game respects **real-life time**, not constant attention.

- Short sessions remain meaningful
- Long absences are survivable, but consequential
- Progress is measured in planning, not clicks

The game rewards:
- foresight
- patience
- adaptation

Not constant activity.

---

## Design Guarantees

- Time always advances
- Absence is meaningful but not fatal by default
- Large changes take time
- Small actions compound over time
- History emerges naturally

---

## Design Intent Summary

Time is the silent force behind the universe.

It:
- gives weight to decisions
- creates consequence
- enables long-term story
- prevents the game from becoming a clicker

The universe does not wait.
Players learn when to act — and when to endure.
