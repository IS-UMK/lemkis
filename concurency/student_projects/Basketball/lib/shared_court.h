#pragma once
#include <pthread.h>

#include <atomic>

const int k_unassigned_id = -1;

/// @brief Data structure used in shared memory for inter-process communication
/// about court state.
struct shared_court_data {
    std::atomic<bool> is_active;
    int id;
    int team1_id = k_unassigned_id;
    int team2_id = k_unassigned_id;

    pthread_mutex_t lock;
    bool is_playing = false;
    int* player_ids;
    int max_players;
    int player_count = 0;
};
