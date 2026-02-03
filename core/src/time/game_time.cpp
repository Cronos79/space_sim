#include "time/game_time.h"

#include <chrono>
#include <iomanip>
#include <sstream>

namespace time_sim {

using namespace std::chrono;

int64_t days_to_seconds(int64_t days)     { return days * 86400; }
int64_t hours_to_seconds(int64_t hours)   { return hours * 3600; }
int64_t minutes_to_seconds(int64_t mins)  { return mins * 60; }

std::string format_gst_datetime(const GameTimeConfig& cfg, int64_t game_seconds_since_start) {
    // C++20 chrono calendar types
    const year_month_day start_ymd{year{cfg.start_year}, month{cfg.start_month}, day{cfg.start_day}};
    const sys_days start_days{start_ymd};

    const sys_seconds start_tp = sys_seconds{start_days}; // midnight
    const sys_seconds now_tp = start_tp + seconds{game_seconds_since_start};

    // Convert back to calendar date + time-of-day
    const auto now_days = floor<days>(now_tp);
    const year_month_day ymd{now_days};
    const auto tod = hh_mm_ss{now_tp - now_days};

    std::ostringstream out;
    out << "GST "
        << int(ymd.year()) << "-"
        << std::setw(2) << std::setfill('0') << unsigned(ymd.month()) << "-"
        << std::setw(2) << std::setfill('0') << unsigned(ymd.day()) << " "
        << std::setw(2) << std::setfill('0') << tod.hours().count() << ":"
        << std::setw(2) << std::setfill('0') << tod.minutes().count();
    return out.str();
}

} // namespace time_sim
