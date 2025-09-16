#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <print>
#include <random>
#include <thread>
#include <vector>

#include "shared_court.h"
#include "utility.h"

// extern std::atomic<bool> shutdown_requested;

class player {
  public:
    static constexpr int k_no_court = -1;
    static constexpr int k_max_random_value = 9;
    static constexpr int k_sleep_duration_ms = 100;
    static constexpr int k_min_delay_ms = 1000;
    static constexpr int k_max_delay_ms = 5000;

    int id;
    int team_id;
    shared_court_data* courts;
    int num_courts;
    int current_court_id{k_no_court};
    bool is_playing = false;

    /// @brief Constructs a player with assigned ID and references to courts.
    player(int pid, int tid, shared_court_data* shared_courts, int count)
        : id(pid), team_id(tid), courts(shared_courts), num_courts(count) {}

    /// @brief Logs that the player is playing on a court.
    void play() const {
        std::println(
            "[player {}] is now playing on court {}", id, current_court_id);
    }

    [[nodiscard]] auto has_active_court() const -> bool {
        return current_court_id != k_no_court;
    }

    /// @brief Attempts to find and join a court.
    auto try_get_court() -> int {
        if (has_active_court()) { return k_no_court; }
        return find_available_court();
    }

    /// @brief Finds the first available court.
    auto find_available_court() -> int {
        for (int i = 0; i < num_courts; ++i) {
            if (process_court(courts[i])) { return courts[i].id; }
        }
        return k_no_court;
    }

    /// @brief Processes a court to check availability.
    auto process_court(shared_court_data& court) -> bool {
        if (court.is_playing) { return false; }
        return try_join_court(court);
    }

    /// @brief Tries to join a given court.
    auto try_join_court(shared_court_data& court) -> bool {
        if (!court.is_active.load()) { return false; }

        pthread_mutex_lock(&court.lock);
        const bool result = attempt_join_court(court);
        pthread_mutex_unlock(&court.lock);
        return result;
    }

    /// @brief Attempts to join court if player's team fits.
    auto attempt_join_court(shared_court_data& court) -> bool {
        if (court.is_playing) { return false; }
        assign_team_if_needed(court);
        return try_add_player_to_court(court);
    }

    void assign_team_if_needed(shared_court_data& court) const {
        if (court.team1_id == k_no_court) {
            court.team1_id = team_id;
        } else if (court.team2_id == k_no_court && court.team1_id != team_id) {
            court.team2_id = team_id;
        }
    }

    auto try_add_player_to_court(shared_court_data& court) -> bool {
        if (!is_player_in_court_team(court)) { return false; }
        return add_player_to_court(court);
    }

    [[nodiscard]] auto is_player_in_court_team(
        const shared_court_data& court) const -> bool {
        return court.team1_id == team_id || court.team2_id == team_id;
    }

    auto add_player_to_court(shared_court_data& court) -> bool {
        if (court.player_count >= court.max_players) { return false; }

        court.player_ids[court.player_count++] = id;
        current_court_id = court.id;
        print_join_message(court);

        check_and_start_match(court);
        return true;
    }

    void print_join_message(const shared_court_data& court) const {
        std::println(
            "[player {} from team {}] joined court {}", id, team_id, court.id);
    }

    void check_and_start_match(shared_court_data& court) const {
        if (court.player_count == court.max_players && !court.is_playing) {
            std::println("[player {}] court {} is full. Starting the match!",
                         id,
                         court.id);
            court.is_playing = true;
        }
    }

    void print_match_start_message(int opponent) const {
        std::println(
            "[player {}] is starting a match against team {}", id, opponent);
    }

    /// @brief Simulates match activity with random duration.
    static void play_match_simulation() {
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<> dis(0, k_max_random_value);

        while (dis(rng) != 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(k_sleep_duration_ms));
        }
    }

    /// @brief Finalizes the match and updates court state.
    void finish_match(int opponent) {
        std::println(
            "[player {}] finished match against team {}", id, opponent);
        courts[current_court_id].player_count--;
        while (courts[current_court_id].player_count > 0) {
            std::this_thread::sleep_for(
                std::chrono::milliseconds(k_sleep_duration_ms));
        }
        is_playing = false;
        current_court_id = k_no_court;
    }

    /// @brief Runs a match against the given opponent.
    void match(int opponent) {
        is_playing = true;
        print_match_start_message(opponent);
        play_match_simulation();
        finish_match(opponent);
    }

    void notify_players_to_play(int court_id) const {
        pthread_mutex_lock(&courts[court_id].lock);
        if (!courts[court_id].is_playing) {
            std::println("[player {}] court {} is full. Starting the match!",
                         id,
                         court_id);
            courts[court_id].is_playing = true;
        }
        pthread_mutex_unlock(&courts[court_id].lock);
    }

    /// @brief Processes finding a court and playing if ready.
    void process_court_selection() {
        try_get_court();
        if (current_court_id >= 0 && courts[current_court_id].is_playing) {
            const int opponent = courts[current_court_id].team1_id == team_id
                                     ? courts[current_court_id].team2_id
                                     : courts[current_court_id].team1_id;
            match(opponent);
        }
    }

    /// @brief Main player thread loop that runs until shutdown.
    void run_player_loop() {
        std::mt19937 gen(std::random_device{}());
        std::uniform_int_distribution<> delay(k_min_delay_ms, k_max_delay_ms);

        while (!utility::shutdown_requested) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay(gen)));
            process_court_selection();
        }

        std::println("[player {}] exiting", id);
    }

    /// @brief Starts player logic in a detached thread.
    void start() {
        std::thread([this]() { run_player_loop(); }).detach();
    }
};