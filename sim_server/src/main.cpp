#include <unistd.h>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <string>
#include <thread>

int main(int argc, char** argv) {
    int tick_ms = 1000;

    // trivial arg parsing: --tick-ms N
    for (int i = 1; i + 1 < argc; ++i) {
        if (std::string(argv[i]) == "--tick-ms") {
            tick_ms = std::atoi(argv[i + 1]);
        }
    }

    std::cout << "space_sim sim_server starting...\n";
    std::cout << "tick_ms=" << tick_ms << "\n";
    std::cout << "pid=" << ::getpid() << "\n";

    using clock = std::chrono::steady_clock;
    auto next = clock::now();

    while (true) {
        next += std::chrono::milliseconds(tick_ms);

        // TODO: process due events/jobs here

        std::cout << "tick\n";
        std::this_thread::sleep_until(next);
    }

    return 0;
}
