#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <print>


constexpr int n_groups = 3;
constexpr int processes_per_group = 3;
constexpr int max_iterations = 5;

constexpr useconds_t local_section_min = 100000;
constexpr useconds_t local_section_range = 200000;
constexpr useconds_t compute_min = 200000;
constexpr useconds_t compute_range = 200000;
constexpr useconds_t sleep_short = 10000;
constexpr int group_none = -1;
constexpr unsigned int sem_shared = 1;
constexpr unsigned int sem_init_value = 1;

class shared_data {
  public:
    sem_t sem;
    int current_group{group_none};
    int active_count{0};
    auto not_my_group(int group_id) -> bool {
        sem_wait(&sem);
        if (current_group == group_none) { current_group = group_id; }
        if (current_group == group_id) {
            active_count++;
            sem_post(&sem);
            return false;
        }
        sem_post(&sem);
        return true;
    }
    auto notify_done() -> void {
        sem_wait(&sem);
        active_count--;
        if (active_count == 0) { current_group = group_none; }
        sem_post(&sem);
    }
};

class group_sync {
    static auto local_section(int group_id, int proc_id) -> void {
        usleep(local_section_min + (rand() % local_section_range));
        std::println(
            "[Group {}] Process {} finished local section", group_id, proc_id);
    }

    static auto compute(int group_id, int proc_id) -> void {
        std::println(
            "[Group {}] Process {} entered compute()", group_id, proc_id);
        usleep(compute_min + (rand() % compute_range));
        std::println(
            "[Group {}] Process {} exiting compute()", group_id, proc_id);
    }

  public:
    static auto group_process(int group_id, int proc_id, shared_data& data)
        -> void {
        for (int i = 0; i < max_iterations; ++i) {
            local_section(group_id, proc_id);
            while (data.not_my_group(group_id)) { usleep(sleep_short); }
            compute(group_id, proc_id);
            data.notify_done();
            while (data.current_group == group_id) { usleep(sleep_short); }
        }
        exit(0);
    }

    static auto create_shared_data() -> shared_data& {
        const int s = sizeof(shared_data);
        const int fd = shm_open("/sync_mem", O_CREAT | O_RDWR, 0666);
        ftruncate(fd, s);
        shared_data& data = *static_cast<shared_data*>(
            mmap(nullptr, s, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0));
        close(fd);
        sem_init(&data.sem, sem_shared, sem_init_value);
        return data;
    }

    static auto destroy_shared_data(shared_data& data) -> void {
        sem_destroy(&data.sem);
        munmap(&data, sizeof(shared_data));
        shm_unlink("/sync_mem");
    }

    static auto run_process(int group, int proc, shared_data& data) -> void {
        const pid_t pid = fork();
        if (pid == 0) { group_sync::group_process(group, proc, data); }
    }

    static auto run_processes(shared_data& data) -> void {
        srand(time(nullptr));
        for (int group = 0; group < n_groups; ++group) {
            for (int proc = 0; proc < processes_per_group; ++proc) {
                run_process(group, proc, data);
            }
        }
        for (int i = 0; i < n_groups * processes_per_group; ++i) {
            wait(nullptr);
        }
    }
};

auto main() noexcept -> int {
    try {
        shared_data& data = group_sync::create_shared_data();
        group_sync::run_processes(data);
        group_sync::destroy_shared_data(data);
        return EXIT_SUCCESS;
    } catch (...) {
        std::println("Exception occured");
        return EXIT_FAILURE;
    }
}
