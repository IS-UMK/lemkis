#include "worker.h"

#include "pipes.h"
#include "prelude.h"
#include "semaphores.h"
#include "unit.h"

void consume(bool& start_first, int id, unit rcv) {
    std::println(
        "[Worker {}] Received and consumed unit of type {} produced by "
        "Worker {}",
        id,
        unit_type_to_string(rcv.type),
        rcv.producer_id);
    usleep(sleep_long);
    start_first = true;
}

void sent_unit(int id) {
    sem_wait(ready[id]);
    const unit u = produce(id);
    write_unit(wt_helper[id][write_fd], u);
    std::println("[Worker {}] Sent unit to Helper", id);
    usleep(sleep_long);
}

void process_loop(bool& start_first, int id) {
    unit rcv;
    if (start_first) { sent_unit(id); }

    if (read_unit(ht_worker[id][read_fd], rcv)) {
        consume(start_first, id, rcv);
    }
    usleep(sleep_short);
}

void worker_process(int id) {
    bool start_first = id % number_two != 0;
    close(wt_helper[id][read_fd]);
    close(ht_worker[id][write_fd]);
    srand(time(nullptr) + getpid());
    while (true) { process_loop(start_first, id); }
}
