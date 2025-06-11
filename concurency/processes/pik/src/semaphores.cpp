#include "semaphores.h"

std::array<sem_t *, number_of_workers> ready;

void init_semaphores() {
    for (int i = 0; i < number_of_workers; ++i) {
        const std::string sem_name = "/ready" + std::to_string(i);
        sem_unlink(sem_name.c_str());
        ready[i] = sem_open(sem_name.c_str(), O_CREAT, chmod, sem_open_value);
        if (ready[i] == SEM_FAILED) {
            perror("sem_open");
            exit(sem_open_value);
        }
    }
}

void cleanup_semaphores() {
    for (int i = 0; i < number_of_workers; ++i) {
        sem_close(ready[i]);
        const std::string sem_name = "/ready" + std::to_string(i);
        sem_unlink(sem_name.c_str());
    }
}
