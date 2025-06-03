#include "matchManager.h"

const int number_of_courts = 1;
const int number_of_teams = 8;
const int number_of_players = 2;
const int exit_code = 0;
const int exit_code_error = 1;

auto main() -> int {
    try {
        match_manager(number_of_courts, number_of_teams, number_of_players)
            .run();
    } catch (const std::exception& e) { return exit_code_error; } catch (...) {
        return exit_code_error;
    }
    return exit_code;
}
