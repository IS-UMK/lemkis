#pragma once

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include <chrono>
#include <csignal>
#include <cstring>
#include <iostream>
#include <print>
#include <thread>

#include "shared_court.h"

/// @brief Manages a court where matches are held, each as a separate process.
class court {
  public:
    static constexpr int k_unassigned_id = -1;
    static constexpr int k_sleep_duration_seconds = 1;

    int id;
    pid_t pid{k_unassigned_id};
    shared_court_data* shared_data;

    court(int court_id, shared_court_data* shared)
        : id(court_id), shared_data(shared) {}

    /// @brief Forks a process to handle match logic.
    void start_process() {
        pid = fork();
        if (pid == 0) {
            run();
            exit(0);
        }
    }

    /// @brief Main court loop running in child process.
    void run() const {
        std::println("court {} process started (PID: {})", id, getpid());
        shared_data[id].is_active = true;

        while (true) {
            if (shared_data[id].is_playing) { start_match(); }
            std::this_thread::sleep_for(
                std::chrono::seconds(k_sleep_duration_seconds));
        }
    }

    /// @brief Stops the court process.
    void stop() const {  // Made const
        if (pid > 0) {
            kill(pid, SIGTERM);
            waitpid(pid, nullptr, 0);
            std::println("court {} process (PID: {}) stopped", id, pid);
        }
    }

    void reset_mutex() const {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&shared_data[id].lock, &attr);
        pthread_mutex_unlock(&shared_data[id].lock);
    }

    void clear_player_data() const {
        if (shared_data[id].player_ids != nullptr) {
            free(shared_data[id].player_ids);
            shared_data[id].player_ids = nullptr;
        }
        shared_data[id].player_ids = static_cast<int*>(
            malloc(sizeof(int) * shared_data[id].max_players));
    }

    void reset_court_state() const {
        shared_data[id].team1_id = k_unassigned_id;
        shared_data[id].team2_id = k_unassigned_id;
        shared_data[id].player_count = 0;
        shared_data[id].is_playing = false;
    }

    /// @brief Resets court to initial state.
    void reset() const {  // Made const and split into smaller functions
        pthread_mutex_unlock(&shared_data[id].lock);
        shared_data[id].is_active = true;

        reset_court_state();
        clear_player_data();
        pthread_mutex_destroy(&shared_data[id].lock);
        reset_mutex();
        std::println("court {} has been reset", id);
    }

    void print_players_remaining() const {
        std::println(
            "{} players left on court {}.", shared_data[id].player_count, id);
    }

    void check_match_completion() const {
        if (shared_data[id].is_playing && shared_data[id].player_count == 0) {
            reset();
        }
    }

    static void sleep() {
        std::this_thread::sleep_for(
            std::chrono::seconds(k_sleep_duration_seconds));
    }

    /// @brief Starts monitoring for match completion.
    void start_match() const {
        while (true) {
            pthread_mutex_lock(&shared_data[id].lock);

            check_match_completion();
            print_players_remaining();

            pthread_mutex_unlock(&shared_data[id].lock);
            sleep();
        }
    }
};