#pragma once

#include <semaphore.h>
#include <sys/mman.h>
#include <unistd.h>

#include <atomic>
#include <csignal>
#include <memory>
#include <string>
#include <vector>

#include "player.h"

class team {
  public:
    int id;
    std::vector<player> players;
    std::string shm_flag_name;
    int* flags;
    sem_t* sem;
    sem_t* match_mutex;
    std::atomic<bool> match_active;
    std::vector<pid_t> player_pids;
    const int process_chmod = 0666;
    const int process_on = 1;
    const int process_off = 0;
    const int player_number = 100;

    void create_mutex(const int player_count) {
        const std::string mutex_name = "/team_mutex_" + std::to_string(id);
        sem_unlink(mutex_name.c_str());
        match_mutex =
            sem_open(mutex_name.c_str(), O_CREAT, process_chmod, process_on);

        for (int i = 0; i < player_count; ++i) {
            players.emplace_back(i + id * player_number);
        }
    }

    auto create_flag(const int player_count) -> int {
        shm_flag_name = "/team_flags_" + std::to_string(id);
        const int fd =
            shm_open(shm_flag_name.c_str(), O_CREAT | O_RDWR, process_chmod);
        ftruncate(fd, static_cast<off_t>(sizeof(int) * player_count));
        return fd;
    }

    auto make_flag(const int player_count) -> void {
        flags = static_cast<int*>(mmap(nullptr,
                                       sizeof(int) * player_count,
                                       PROT_READ | PROT_WRITE,
                                       MAP_SHARED,
                                       create_flag(player_count),
                                       process_off));
    }

    team(int id, int player_count) : id(id), match_active(false) {
        create_mutex(player_count);
        create_flag(player_count);
        make_flag(player_count);

        sem = sem_open(("/team_sem_" + std::to_string(id)).c_str(),
                       O_CREAT,
                       process_chmod,
                       process_off);
    }

    void reset_flags() const {
        for (size_t i = 0; i < players.size(); ++i) { flags[i] = process_off; }
    }

    void set_player_pid(int i, pid_t pid) {
        players[i].pid = pid;
        player_pids.push_back(pid);
    }

    void start_players_creator(int dur) {
        for (size_t i = 0; i < players.size(); ++i) {
            const pid_t pid = fork();
            if (pid == process_off) {
                players[i].play(dur, sem, &flags[i], &match_active);
                exit(0);
            } else {
                set_player_pid(static_cast<int>(i), pid);
            }
        }
    }

    void start_players(int dur) {
        match_active = true;
        reset_flags();
        player_pids.clear();
        start_players_creator(dur);
    }
    [[nodiscard]]
    auto is_abandoned() const -> bool {
        for (size_t i = 0; i < players.size(); ++i) {
            if (flags[i] == process_off) { return false; };
        }
        return true;
    }

    void stop_players() {
        match_active = false;
        for (const pid_t pid : player_pids) { kill(pid, SIGTERM); }
        // Reset semaphore
        int val;
        sem_getvalue(sem, &val);
        for (int i = 0; i < val; ++i) { sem_wait(sem); }
    }

    ~team() {
        sem_close(sem);
        munmap(flags, sizeof(int) * players.size());
        shm_unlink(shm_flag_name.c_str());
        sem_close(match_mutex);
    }
};