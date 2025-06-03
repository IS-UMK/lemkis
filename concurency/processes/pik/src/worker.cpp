#include "worker.h"

#include "pipes.h"
#include "prelude.h"
#include "semaphores.h"
#include "unit.h"

auto check_message(int id, unit rcv) -> bool {
    if (rcv.is_request) {
        std::println("[Worker {}] Got request to produce", id);
    } else {
        std::println("[Worker {}] Received and consumed unit of type {}",
                     id,
                     unit_type_to_string(rcv.type));
    }
    usleep(sleep_long);
    return !rcv.is_request;
}

void sent_unit(int id) {
    sem_wait(ready[id]);
    const unit u = produce(id);
    write_unit(wt_helper[id][write_fd], u);
    std::println("[Worker {}] Sent unit to Helper", id);
    usleep(sleep_long);
}

void sent_signal(int id) {
    sem_wait(ready[id]);
    const unit u{static_cast<UnitType>(0), id, true, true};
    write_unit(wt_helper[id][write_fd], u);
    std::println("[Worker {}] Sent Request signal", id);
    usleep(sleep_long);
}

void process_loop_worker(int id, bool& is_signal, unit& rcv) {
    if (!is_signal) { sent_unit(id); }
    if (read_unit(ht_worker[id][read_fd], rcv)) {
        is_signal = check_message(id, rcv);
    }
    if (is_signal) { sent_signal(id); }
}

void worker_process(int id) {
    close(wt_helper[id][read_fd]);
    close(ht_worker[id][write_fd]);
    srand(time(nullptr) + getpid());
    bool is_signal = id % number_two == 0;
    unit rcv;
    while (true) { process_loop_worker(id, is_signal, rcv); }
}
