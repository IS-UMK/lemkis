#include "pipes.h"

#include "prelude.h"

std::array<std::array<int, pipes_ends_number>, number_of_workers> wt_helper;
std::array<std::array<int, pipes_ends_number>, number_of_workers> ht_worker;
std::array<std::array<int, pipes_ends_number>, number_of_workers> ht_helper;

void create_pipes() {
    for (int i = 0; i < number_of_workers; ++i) {
        pipe(wt_helper[i].data());
        pipe(ht_worker[i].data());
        pipe(ht_helper[i].data());

        fcntl(wt_helper[i][read_fd], F_SETFL, O_NONBLOCK);
        fcntl(ht_helper[i][read_fd], F_SETFL, O_NONBLOCK);
    }
}

void close_pipes_parent() {
    for (int i = 0; i < number_of_workers; ++i) {
        close(wt_helper[i][read_fd]);
        close(wt_helper[i][write_fd]);
        close(ht_worker[i][read_fd]);
        close(ht_worker[i][write_fd]);
        close(ht_helper[i][read_fd]);
        close(ht_helper[i][write_fd]);
    }
}
