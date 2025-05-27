#include "worker.h"
#include "prelude.h"
#include "pipes.h"
#include "semaphores.h"
#include "unit.h"

void worker_process(int id) {
    bool start_first = id % number_two != 0;
    unit rcv;
    close(wt_helper[id][read_fd]);
    close(ht_worker[id][write_fd]);
    srand(time(nullptr) + getpid());
    while (true) {
        if (start_first) {
            sem_wait(ready[id]);
            const unit u = produce(id);
            write_unit(wt_helper[id][write_fd], u);
            std::println("[Worker {}] Sent unit to Helper", id);
            usleep(sleep_long);
        }

        if (read_unit(ht_worker[id][read_fd], rcv)) {
            std::println("[Worker {}] Received and consumed unit of type {} produced by Worker {}", id, unit_type_to_string(rcv.type), rcv.producer_id);
            usleep(sleep_long);
            start_first = true;
        }
        usleep(sleep_short);
    }
}