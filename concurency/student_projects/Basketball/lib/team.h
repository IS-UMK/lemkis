#pragma once
#include <bits/this_thread_sleep.h>

#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <vector>

#include "player.h"

class team {
  public:
    int id;
    mutable std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::shared_ptr<player>> players;
    bool ready = false, in_match = false;
    const int oneHundred = 100;

    team(int id, int count) : id(id) {
        for (int i = 0; i < count; ++i) {
            players.emplace_back(std::make_shared<player>(i + id * oneHundred));
        };
    }

    void arrive() {
        std::this_thread::sleep_for(std::chrono::milliseconds(oneHundred));
        const std::lock_guard<std::mutex> lg(mtx);
        ready = true;
        cv.notify_all();
    }

    void wait_ready() {
        std::unique_lock<std::mutex> ul(mtx);
        cv.wait(ul, [this] { return ready && !in_match; });
        in_match = true;
    }

    void finish() {
        const std::lock_guard<std::mutex> lg(mtx);
        in_match = ready = false;
        for (auto& p : players) { p->reset(); };
    }

    auto all_abandoned() const -> bool {
        return std::all_of(players.begin(), players.end(), [](const auto& p) {
            const std::lock_guard<std::mutex> lg(p->mtx);
            return p->abandoned;
        });
    }
};