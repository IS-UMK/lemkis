#include "helper.h"
#include "prelude.h"
#include "pipes.h"
#include "semaphores.h"
#include "unit.h"

void helper_process(int id) {
    const int next = (id + 1) % number_of_workers;
    unit buffer{0, 0, false};
    close(wt_helper[id][write_fd]);
    close(ht_worker[id][read_fd]);
    close(ht_helper[id][write_fd]);
    close(ht_helper[next][read_fd]);
    while (true) {
        unit u;
        if (read_unit(wt_helper[id][read_fd], u)) {
            std::println("[Helper {}] Received unit of type {} from its worker", id, unit_type_to_string(u.type));
            usleep(sleep_long);
            if (u.type == id) {
                write_unit(ht_worker[id][write_fd], u);
                std::println("[Helper {}] Sent back unit to its worker", id);
                usleep(sleep_long);
                sem_post(ready[id]);
            } else {
                buffer = u;
            }
        }

        if (buffer.busy) {
            if (sem_trywait(ready[next]) == 0) {
                if (write_unit(ht_helper[next][write_fd], buffer)) {
                    std::println("[Helper {}] Passed unit of type {} to Helper {}", id, unit_type_to_string(buffer.type), next);
                    usleep(sleep_long);
                    sem_post(ready[id]);
                    buffer.busy = false;
                } else {
                    sem_post(ready[next]);
                }
            }
        }

        if (read_unit(ht_helper[id][read_fd], u)) {
            std::println("[Helper {}] Received unit of type {} produced by Worker {}", id, unit_type_to_string(u.type), u.producer_id);
            usleep(sleep_long);
            if (u.type == id) {
                write_unit(ht_worker[id][write_fd], u);
                std::println("[Helper {}] Sent unit of type {} produced by Worker {} to its worker", id, unit_type_to_string(u.type), u.producer_id);
                usleep(sleep_long);
                sem_post(ready[id]);
            } else {
                buffer = u;
            }
        }
        usleep(sleep_short);
    }
}
