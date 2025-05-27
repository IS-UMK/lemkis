#pragma once
#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <cstdio>
#include <iostream>
#include <memory>
#include <mutex>
#include <random>
#include <thread>
#include <vector>


class player {
  public:
    int id;
    mutable std::mutex mtx;
    bool in_match = false, abandoned = false;
    std::mt19937 gen;
    const int timeout = 500;


    explicit player(int id) : id(id), gen(std::random_device{}()) {}

    void arrive() {
        {
            const std::lock_guard<std::mutex> lg(mtx);
            if (abandoned) { return; }
            in_match = true;
        }
    }

    void leave(const int play_time = 0) {
        const std::lock_guard<std::mutex> lg(mtx);
        if (!abandoned) {
            abandoned = true;
            in_match = false;
            std::printf("Player %d abandoned after %dms\n", id, play_time);
        }
    }

    auto start(int dur) -> int {
        std::uniform_int_distribution<> dist(timeout, dur);
        const int play_time = dist(gen);

        std::this_thread::sleep_for(std::chrono::milliseconds(play_time));
        return play_time;
    }

    void play(int dur, std::shared_ptr<std::atomic<bool>> &over) {
        arrive();
        std::thread([this, dur, over]() {
            auto play_time = start(dur);
            if (*over) { return; }
            leave(play_time);
        }).detach();
    }

    auto available() const -> bool {
        const std::lock_guard<std::mutex> lg(mtx);
        return !in_match && !abandoned;
    }

    void reset() {
        const std::lock_guard<std::mutex> lg(mtx);
        abandoned = in_match = false;
    }
};