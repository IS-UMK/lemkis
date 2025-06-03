#pragma once

#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <print>
#include <random>
#include <string>
#include <thread>
#include <utility>
#include <vector>

class player {
  public:
    int id;
    std::string shm_name;
    std::mt19937 gen;
    const int timeout = 500;
    const int delay = 100;
    const int flag_value = 1;
    std::atomic<bool>* match_active{nullptr};
    pid_t pid{0};

    explicit player(int id)
        : id(id),
          gen(std::random_device{}()),
          shm_name("/player_" + std::to_string(id)) {}

    auto start(int dur, std::atomic<bool>* active) {
        match_active = active;
        shm_unlink(shm_name.c_str());
        std::uniform_int_distribution<> dist(timeout, dur);
        return dist(gen);
    }

    auto match_while(int play_time) -> void {
        auto start = std::chrono::steady_clock::now();
        while (*match_active &&
               std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - start)
                       .count() < play_time) {
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
        }
    }


    void play(int dur,
              sem_t* sem,
              int* abandoned_flag,
              std::atomic<bool>* active) {
        auto play_time = start(dur, active);

        match_while(play_time);

        if (*match_active) {
            *abandoned_flag = flag_value;
            std::println("Player {} abandoned after {}ms", id, play_time);
            sem_post(sem);
        }
    }

    ~player() { shm_unlink(shm_name.c_str()); }
};