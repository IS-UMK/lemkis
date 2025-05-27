#pragma once
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <random>
#include <thread>
#include <tuple>
#include <vector>

#include "team.h"

class match_manager {
    std::vector<std::shared_ptr<team>> teams;
    std::mutex court_mtx;
    std::mutex cout_mtx;
    std::condition_variable court_cv;
    std::queue<int> courts;
    const int magicNumber = 1;
    const int oneHundred = 100;
    const int dur = 5000;

    auto match_init(int t1, int t2) -> int {
        teams[t1]->wait_ready();
        teams[t2]->wait_ready();

        std::unique_lock<std::mutex> ul(court_mtx);
        court_cv.wait(ul, [this] { return !courts.empty(); });
        const int c = courts.front();
        courts.pop();
        return c;
    }

    static void print_match_start(int court,
                                  std::shared_ptr<team> const &t1,
                                  std::shared_ptr<team> const &t2) {
        std::printf("Court %d: Match started between Team %d and Team %d\n",
                    court,
                    t1->id,
                    t2->id);
    }

    static void print_match_end(int court,
                                std::shared_ptr<team> const &t1,
                                std::shared_ptr<team> const &t2) {
        std::printf("Court %d: Match finished between Team %d and Team %d\n",
                    court,
                    t1->id,
                    t2->id);
    }

    static void print_match_abandoned(int court,
                                      std::shared_ptr<team> const &t1,
                                      std::shared_ptr<team> const &t2) {
        std::printf("Court %d: Match abandoned by Team %d\n",
                    court,
                    t1->all_abandoned() ? t1->id : t2->id);
    }

    void start_teams(int t1, int t2, std::shared_ptr<std::atomic<bool>> over) {

        for (auto &p : teams[t1]->players) { p->play(dur, over); };
        for (auto &p : teams[t2]->players) { p->play(dur, over); };
    }

    void match_loop(int t1,
                    int t2,
                    int court,
                    std::shared_ptr<std::atomic<bool>> const &over) {
        while (!*over) {
            std::this_thread::sleep_for(std::chrono::milliseconds(oneHundred));
            if (teams[t1]->all_abandoned() || teams[t2]->all_abandoned()) {
                *over = true;
                print_match_abandoned(court, teams[t1], teams[t2]);
                break;
            }
        }
    }

    void start_timer(std::shared_ptr<std::atomic<bool>> const &over,
                     int t1,
                     int t2,
                     int court) {
        std::thread timer([over, duration = dur]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(duration));
            *over = true;
        });
        match_loop(t1, t2, court, over);
        timer.detach();
    }

    void stop_teams(int t1, int t2) {
        teams[t1]->finish();
        teams[t2]->finish();
        std::thread([this, t1] { teams[t1]->arrive(); }).detach();
        std::thread([this, t2] { teams[t2]->arrive(); }).detach();
    }

    void finish_match(int t1, int t2, int court) {
        {
            const std::lock_guard<std::mutex> lg(court_mtx);
            courts.push(court);
        }
        court_cv.notify_one();
        stop_teams(t1, t2);
    }

    void play_match(int t1, int t2) {
        auto over = std::make_shared<std::atomic<bool>>(false);
        const int court = match_init(t1, t2);
        print_match_start(court, teams[t1], teams[t2]);
        start_teams(t1, t2, over);
        start_timer(over, t1, t2, court);
        print_match_end(court, teams[t1], teams[t2]);
        finish_match(t1, t2, court);
    }

    void combine_teams() {
        std::vector<std::thread> matches;
        for (size_t i = 0; i < teams.size(); ++i) {
            for (size_t j = i + magicNumber; j < teams.size(); ++j) {
                matches.emplace_back(&match_manager::play_match, this, i, j);
            }
        }
        for (auto &m : matches) { m.join(); };
    }

  public:
    match_manager(int court_count, int team_count, int player_count) {
        for (int i = 0; i < team_count; ++i) {
            teams.emplace_back(
                std::make_shared<team>(i + magicNumber, player_count));
        }
        for (int i = 0; i < court_count; ++i) { courts.push(i); };
    }

    void run() {
        std::vector<std::thread> arrivals;
        arrivals.reserve(teams.size());
        for (auto &t : teams) { arrivals.emplace_back(&team::arrive, t); };
        for (auto &t : arrivals) { t.join(); };
        combine_teams();
    }
};
