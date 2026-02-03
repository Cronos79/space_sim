#include "time/duration.h"

#include <sstream>

namespace time_sim {

std::string format_duration(int64_t seconds) {
    if (seconds <= 0) return "0s";

    int64_t s = seconds;
    int64_t days = s / 86400; s %= 86400;
    int64_t hours = s / 3600; s %= 3600;
    int64_t minutes = s / 60; s %= 60;

    std::ostringstream out;
    bool first = true;

    auto emit = [&](int64_t v, const char* label) {
        if (v <= 0) return;
        if (!first) out << " ";
        out << v << label;
        first = false;
    };

    emit(days, "d");
    emit(hours, "h");
    emit(minutes, "m");
    emit(s, "s");

    return out.str();
}

} // namespace time_sim
