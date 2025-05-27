#include "matchManager.h"

const int global_magic_number_of_courts = 10;
const int global_magic_number_of_teams = 4;
const int global_magic_number_of_players = 2;

auto main() -> int {
    match_manager(global_magic_number_of_courts,
                  global_magic_number_of_teams,
                  global_magic_number_of_players)
        .run();
    return 0;
}