#pragma once
#include <cstdint>
#include <string>

namespace time_sim {

struct GameTimeConfig {
    // Start date (Galactic Standard Time) in Gregorian calendar.
    int start_year = 2350;
    unsigned start_month = 1;
    unsigned start_day = 1;

    // Scale: how many in-game seconds per real second.
    // Your design: 36 game seconds per real second.
    double game_seconds_per_real_second = 36.0;

    // Fixed sim step in game seconds (e.g., 10 minutes).
    int64_t tick_step_game_seconds = 600;
};

// Convert game_seconds_since_start into a readable datetime string.
// Example: "GST 2350-01-03 12:40"
std::string format_gst_datetime(const GameTimeConfig& cfg, int64_t game_seconds_since_start);

// Convenience helpers
int64_t days_to_seconds(int64_t days);
int64_t hours_to_seconds(int64_t hours);
int64_t minutes_to_seconds(int64_t minutes);

} // namespace time_sim
