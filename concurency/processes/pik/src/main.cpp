#include "helper.h"
#include "pipes.h"
#include "prelude.h"
#include "semaphores.h"
#include "worker.h"

void end_program() {
    close_pipes_parent();
    for (int i = 0; i < number_of_all_processe; ++i) { wait(nullptr); }
    cleanup_semaphores();
}

// Create and start Helper processes
void create_helpers() {
    for (int i = 0; i < number_of_workers; ++i) {
        const pid_t pid = fork();
        if (pid == 0) {
            Helper(i).run();
            exit(0);
        }
    }
}

// Create and start Worker processes
void create_workers() {
    for (int i = 0; i < number_of_workers; ++i) {
        const pid_t pid = fork();
        if (pid == 0) {
            Worker(i).run();
            exit(0);
        }
    }
}

auto main() -> int {
    create_pipes();
    init_semaphores();
    create_helpers();
    create_workers();
    end_program();
    return 0;
}
