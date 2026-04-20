#include <atomic>
#include <csignal>
#include <exception>
#include <iostream>

#include "matchManager.h"
#include "utility.h"


namespace {
    constexpr int k_number_of_teams = 4;
    constexpr int k_number_of_players = 5;
    constexpr int k_number_of_courts = 1;
    constexpr int k_error_code = -1;
    constexpr std::chrono::seconds k_sleep_duration(1);
}  // namespace


/// @brief Runs the entire match simulation.
void static run_simulation() {
    match_manager manager(
        k_number_of_teams, k_number_of_players, k_number_of_courts);
    manager.start();

    while (!utility::shutdown_requested) {
        std::this_thread::sleep_for(k_sleep_duration);
    }

    manager.stop();
}

/// @brief Entry point of the program.
auto main() -> int {
    try {
        utility::setup_signal_handler();
        run_simulation();
        return 0;
    } catch (...) {
        std::cerr << "Unknown error occurred\n";
        return k_error_code;
    }
}