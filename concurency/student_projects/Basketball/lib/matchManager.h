#pragma once

#include <semaphore.h>
#include <sys/wait.h>

#include <iostream>
#include <memory>
#include <print>
#include <queue>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include "team.h"

class match_manager {
    std::vector<std::shared_ptr<team>> teams;
    std::vector<sem_t *> court_sems;
    const int dur = 5000;
    const int process_chmod = 0666;
    const int process_on = 1;
    const int delay = 100;

  public:
    auto create_court_sem(int court_count) -> void {
        for (int i = 0; i < court_count; ++i) {
            auto name = "/court_sem_" + std::to_string(i);
            sem_unlink(name.c_str());
            court_sems.push_back(
                sem_open(name.c_str(), O_CREAT, process_chmod, process_on));
        }
    }

    match_manager(int court_count, int team_count, int player_count) {
        create_court_sem(court_count);
        for (int i = 0; i < team_count; ++i) {
            teams.emplace_back(
                std::make_shared<team>(i + process_on, player_count));
        }
    }


    auto start() -> std::queue<std::pair<int, int>> {
        std::queue<std::pair<int, int>> matches;
        for (size_t i = 0; i < teams.size(); ++i) {
            for (size_t j = i + process_on; j < teams.size(); ++j) {
                matches.emplace(i, j);
            }
        }
        return matches;
    }

    void match_fork(std::queue<std::pair<int, int>> &matches, size_t court) {
        auto match = matches.front();
        matches.pop();

        if (fork() == 0) {
            run_match(court, match.first, match.second);
            exit(0);
        }
    }

    auto check(size_t court) -> bool {
        if (court == -process_on) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            return true;
        }
        return false;
    }

    void match_loop(std::queue<std::pair<int, int>> &matches) {
        while (!matches.empty()) {
            const size_t court = find_available_court();
            if (check(court)) { continue; };
            match_fork(matches, court);
        }

        while (wait(nullptr) > 0) {};
    }

    void run() {
        // Create all possible matches
        auto matches = start();

        match_loop(matches);
    }
    ~match_manager() {
        for (auto *sem : court_sems) {
            sem_close(sem);
            sem_unlink("/court_sem_*");
        }
    }

  private:
    auto find_available_court() -> size_t {
        for (size_t k = 0; k < court_sems.size(); ++k) {
            if (sem_trywait(court_sems[k]) == 0) { return k; }
        }
        return -process_on;
    }

    void start_match_players(int team1_idx, int team2_idx, size_t court) {
        sem_wait(teams[team1_idx]->match_mutex);
        sem_wait(teams[team2_idx]->match_mutex);
        std::println("Court {}: Match started between Team {} and Team {}",
                     court,
                     teams[team1_idx]->id,
                     teams[team2_idx]->id);

        teams[team1_idx]->start_players(dur);
        teams[team2_idx]->start_players(dur);
    }

    void print_match_abandoned(size_t court, int team1_idx, int team2_idx) {
        std::println("Court {}: Match abandoned by Team {}",
                     court,
                     teams[team1_idx]->is_abandoned() ? teams[team1_idx]->id
                                                      : teams[team2_idx]->id);
        teams[team1_idx]->stop_players();
        teams[team2_idx]->stop_players();
    }

    void match_loop(size_t court, int team1_idx, int team2_idx) {
        while (true) {
            if (teams[team1_idx]->is_abandoned() ||
                teams[team2_idx]->is_abandoned()) {
                print_match_abandoned(court, team1_idx, team2_idx);
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }

    void end_match(size_t court, int team1_idx, int team2_idx) {
        std::println("Court {}: Match finished between Team {} and Team {}",
                     court,
                     teams[team1_idx]->id,
                     teams[team2_idx]->id);

        sem_post(teams[team1_idx]->match_mutex);
        sem_post(teams[team2_idx]->match_mutex);
        sem_post(court_sems[court]);
    }

    void run_match(size_t court, int team1_idx, int team2_idx) {

        start_match_players(team1_idx, team2_idx, court);

        match_loop(court, team1_idx, team2_idx);
        end_match(court, team1_idx, team2_idx);
    }
};