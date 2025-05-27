#include "prelude.h"
#include "helper.h"
#include "pipes.h"
#include "semaphores.h"
#include "worker.h"

auto main() -> int{
    create_pipes();
    init_semaphores();
    for (int i = 0; i < number_of_workers; ++i) {
        const pid_t pid = fork();
        if (pid == 0) {
            helper_process(i);
            exit(0);
        }
    }
    for (int i = 0; i < number_of_workers; ++i) {
        const pid_t pid = fork();
        if (pid == 0) {
            worker_process(i);
            exit(0);
        }
    }
    close_pipes_parent();
    for (int i = 0; i < number_two * number_of_workers; ++i) { wait(nullptr); }
    cleanup_semaphores();
    return 0;
}
