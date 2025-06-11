#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#include <iostream>
#include <memory>
#include <print>
#include <vector>

#include "court.h"
#include "team.h"

/// @brief The match_manager class handles the lifecycle and coordination of
/// courts and teams,
class match_manager {
  public:
    static constexpr int k_default_file_mode = 0666;
    static constexpr int k_players_per_team_multiplier = 2;
    static constexpr int k_unassigned_id = -1;

    int num_teams;
    int num_players;
    int num_courts;

    shared_court_data* shared_courts{nullptr};
    int shm_fd;

    std::vector<std::shared_ptr<court>> courts;
    std::vector<std::shared_ptr<team>> teams;

    /// @brief Constructs a match_manager with the given number of teams,
    /// players per team, and courts.
    match_manager(int n_teams, int n_players, int n_courts)
        : num_teams(n_teams), num_players(n_players), num_courts(n_courts) {}

    /// @brief Maps the shared memory region into the process’s address space.
    void set_memory() {
        shared_courts = static_cast<shared_court_data*>(
            mmap(nullptr,
                 sizeof(shared_court_data) * num_courts,
                 PROT_READ | PROT_WRITE,
                 MAP_SHARED,
                 shm_fd,
                 0));
    }

    /// @brief Initializes shared memory to store the state of all courts.
    void initialize_shared_memory() {
        shm_fd = shm_open("/court_shm", O_CREAT | O_RDWR, k_default_file_mode);
        const auto shared_mem_size =
            static_cast<off_t>(sizeof(shared_court_data) * num_courts);
        ftruncate(shm_fd, shared_mem_size);
        set_memory();
    }

    /// @brief Initializes all courts with shared memory and launches their
    /// processes.
    void initialize_courts() {
        const int max_players_per_court =
            num_players * k_players_per_team_multiplier;

        for (int i = 0; i < num_courts; ++i) {
            initialize_court(i, max_players_per_court);
        }
    }

    /// @brief Creates a court instance and starts its process.
    void add_court(int court_id, int max_players) {
        shared_courts[court_id].max_players = max_players;
        shared_courts[court_id].player_ids =
            static_cast<int*>(malloc(sizeof(int) * max_players));
        initialize_mutex(court_id);
        auto newcourt = std::make_shared<court>(court_id, shared_courts);
        courts.push_back(newcourt);
        newcourt->start_process();
    }

    /// @brief Initializes shared data and internal state for a specific court.
    void initialize_court(int court_id, int max_players) {
        shared_courts[court_id].id = court_id;
        shared_courts[court_id].is_active = false;
        shared_courts[court_id].team1_id = k_unassigned_id;
        shared_courts[court_id].team2_id = k_unassigned_id;
        shared_courts[court_id].player_count = 0;
        shared_courts[court_id].is_playing = false;

        add_court(court_id, max_players);
    }

    /// @brief Initializes the mutex for accessing a court's shared memory
    /// safely.
    void initialize_mutex(int court_id) const {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&shared_courts[court_id].lock, &attr);
    }

    /// @brief Initializes all teams and starts their internal threads
    /// (players).
    void initialize_teams() {
        for (int i = 0; i < num_teams; ++i) {
            teams.push_back(std::make_shared<team>(
                i, num_players, shared_courts, num_courts));
        }
    }

    /// @brief Starts the full match simulation:
    void start() {
        if(num_teams < 2) {
            std::println("Error: At least 2 teams are required");
            return;
        }
        initialize_shared_memory();
        initialize_courts();
        initialize_teams();
    }

    /// @brief Stops all court processes gracefully.
    void cleanup_courts() {
        for (auto& court : courts) { court->stop(); }
    }

    /// @brief Unmaps and unlinks the shared memory used by courts.
    void cleanup_shared_memory() const {
        munmap(shared_courts, sizeof(shared_court_data) * num_courts);
        close(shm_fd);
        shm_unlink("/court_shm");
    }

    /// @brief Destroys all synchronization primitives and frees dynamically
    /// allocated memory.
    void destroy_mutexes() const {
        for (int i = 0; i < num_courts; ++i) {
            pthread_mutex_destroy(&shared_courts[i].lock);
            free(shared_courts[i].player_ids);
        }
    }

    /// @brief Stops the simulation and cleans up all resources.
    void stop() {
        cleanup_courts();
        destroy_mutexes();
        cleanup_shared_memory();
        std::println("match_manager cleanup complete.");
    }
};