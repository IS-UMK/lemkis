#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>

constexpr int N_GROUPS = 3;
constexpr int PROCESSES_PER_GROUP = 3;
constexpr int MAX_ITERATIONS = 5;


struct SharedData {
    sem_t sem;
    int current_group;
    int active_count;
};


void local_section(int group_id, int proc_id) {
    usleep(100000 + rand() % 200000);
    printf("[Group %d] Process %d finished local section\n", group_id, proc_id);
}


void compute(int group_id, int proc_id) {
    printf("[Group %d] Process %d entered compute()\n", group_id, proc_id);
    usleep(200000 + rand() % 200000);
    printf("[Group %d] Process %d exiting compute()\n", group_id, proc_id);
}


void group_process(int group_id, int proc_id, SharedData& data) {
    for (int i = 0; i < MAX_ITERATIONS; ++i) {
        local_section(group_id, proc_id);

        while (true) {
            sem_wait(&data.sem);
            if (data.current_group == -1) data.current_group = group_id;
            if (data.current_group == group_id) {
                data.active_count++;
                sem_post(&data.sem);
                break;
            }
            sem_post(&data.sem);
            usleep(10000);
        }

        compute(group_id, proc_id);

        sem_wait(&data.sem);
        data.active_count--;
        if (data.active_count == 0) data.current_group = -1;
        sem_post(&data.sem);

        while (data.current_group == group_id) usleep(10000);
    }
    exit(0);
}


int main() {
    srand(time(NULL));

    int fd = shm_open("/sync_mem", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedData));
    SharedData& data = *(SharedData*)mmap(
        nullptr, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    sem_init(&data.sem, 1, 1);
    data.active_count = 0;
    data.current_group = -1;

    for (int group = 0; group < N_GROUPS; ++group) {
        for (int proc = 0; proc < PROCESSES_PER_GROUP; ++proc) {
            pid_t pid = fork();
            if (pid == 0) { group_process(group, proc, data); }
        }
    }

    for (int i = 0; i < N_GROUPS * PROCESSES_PER_GROUP; ++i) { wait(nullptr); }

    sem_destroy(&data.sem);
    munmap(&data, sizeof(SharedData));
    shm_unlink("/sync_mem");

    return 0;
}
