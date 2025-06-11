#pragma once

#include <memory>
#include <vector>

#include "player.h"

/// @brief Represents a team consisting of multiple players.
class team {
  public:
    int id;
    std::vector<std::shared_ptr<player>> players;
    const int additional_player = 1;

    /// @brief Constructs a team with the specified number of players.
    team(int team_id,
         int num_players,
         shared_court_data* courts,
         int num_courts)
        : id(team_id) {
        for (int i = 0; i < num_players; ++i) {
            auto new_player = std::make_shared<player>(
                i + additional_player, team_id, courts, num_courts);
            players.push_back(new_player);
            new_player->start();
        }
    }
};
