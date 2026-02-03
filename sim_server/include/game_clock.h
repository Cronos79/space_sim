#pragma once
#include <cstdint>
#include <chrono>

#include "time/game_time.h"

namespace sim {

class GameClock {
public:
    explicit GameClock(time_sim::GameTimeConfig cfg)
        : cfg_(cfg),
          last_real_(std::chrono::steady_clock::now()) {}

    // Call this frequently (each loop iteration)
    // Advances game time deterministically using fixed tick steps.
    void update() {
        using namespace std::chrono;
        auto now = steady_clock::now();
        duration<double> real_dt = now - last_real_;
        last_real_ = now;

        // Convert real seconds to game seconds (accumulate fractional)
        double add_game = real_dt.count() * cfg_.game_seconds_per_real_second;
        game_accum_ += add_game;

        // Convert to integer game seconds we can commit
        int64_t whole = static_cast<int64_t>(game_accum_);
        if (whole > 0) {
            game_seconds_ += whole;
            game_accum_ -= static_cast<double>(whole);
        }

        // Step the sim in fixed-size game ticks
        while (tick_debt_ + cfg_.tick_step_game_seconds <= game_seconds_) {
            tick_debt_ += cfg_.tick_step_game_seconds;
            ++tick_count_;
            // TODO: call sim step here later (industry, AI, etc.)
        }
    }

    int64_t now_game_seconds() const { return game_seconds_; }
    int64_t tick_count() const { return tick_count_; }
    const time_sim::GameTimeConfig& cfg() const { return cfg_; }

    std::string now_gst() const {
        return time_sim::format_gst_datetime(cfg_, game_seconds_);
    }

private:
    time_sim::GameTimeConfig cfg_;
    std::chrono::steady_clock::time_point last_real_;

    int64_t game_seconds_ = 0;
    double  game_accum_   = 0.0;

    int64_t tick_debt_  = 0;   // how far we have stepped deterministic sim
    int64_t tick_count_ = 0;
};

} // namespace sim
