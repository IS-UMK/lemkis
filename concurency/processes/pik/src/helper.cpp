#include "helper.h"

#include "pipes.h"
#include "prelude.h"
#include "semaphores.h"
#include "unit.h"

void close_unsued_pipe_ends(int id, int next) {
    close(wt_helper[id][write_fd]);
    close(ht_worker[id][read_fd]);
    close(ht_helper[id][write_fd]);
    close(ht_helper[next][read_fd]);
}

void message_send_helper_to_worker(int id, unit u) {
    std::println("[Helper {}] Sent unit {} produced by Worker {} to its worker",
                 id,
                 unit_type_to_string(u.type),
                 u.producer_id);
}
auto check_if_unittype_matches(int id, unit u) -> bool {
    if (u.type == id) {
        write_unit(ht_worker[id][write_fd], u);
        message_send_helper_to_worker(id, u);
        usleep(sleep_long);
        sem_post(ready[id]);
        return true;
    }
    return false;
}

void get_unit_from_worker(int id, unit& buffer) {
    unit u;
    if (read_unit(wt_helper[id][read_fd], u)) {
        std::println("[Helper {}] Received unit of type {} from its worker",
                     id,
                     unit_type_to_string(u.type));
        usleep(sleep_long);
        if (!check_if_unittype_matches(id, u)) { buffer = u; }
    }
}

void message_get_helper_from_helper(int id, unit u) {
    std::println("[Helper {}] Received unit of type {} produced by Worker {}",
                 id,
                 unit_type_to_string(u.type),
                 u.producer_id);
}

void get_unit_from_helper(int id, unit& buffer) {
    unit u;
    if (read_unit(ht_helper[id][read_fd], u)) {
        message_get_helper_from_helper(id, u);
        usleep(sleep_long);
        if (!check_if_unittype_matches(id, u)) { buffer = u; }
    }
}

void message_send_helper_to_helper(int id, unit buffer, int next) {
    std::println("[Helper {}] Passed unit of type {} to Helper {}",
                 id,
                 unit_type_to_string(buffer.type),
                 next);
}

void pass_unit_to_helper(int id, int next, unit& buffer) {
    if (buffer.busy && sem_trywait(ready[next]) == 0) {
        write_unit(ht_helper[next][write_fd], buffer);
        message_send_helper_to_helper(id, buffer, next);
        usleep(sleep_long);
        sem_post(ready[id]);
        buffer.busy = false;
    }
}

void helper_process(int id) {
    const int next = (id + 1) % number_of_workers;
    unit buffer{0, 0, false};
    close_unsued_pipe_ends(id, next);
    while (true) {
        get_unit_from_worker(id, buffer);
        pass_unit_to_helper(id, next, buffer);
        get_unit_from_helper(id, buffer);
        usleep(sleep_short);
    }
}
