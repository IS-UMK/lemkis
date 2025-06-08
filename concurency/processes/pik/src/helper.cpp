#include "helper.h"

#include "pipes.h"
#include "prelude.h"
#include "semaphores.h"
#include "unit.h"

Helper::Helper(int id)
    : id(id), next_id((id + number_one) % number_of_workers) {}

void Helper::run() {
    close_unused_pipes_ends();
    while (true) { processLoop(); }
}

void Helper::close_unused_pipes_ends() const {
    close(wt_helper[id][write_fd]);
    close(ht_worker[id][read_fd]);
    close(ht_helper[id][write_fd]);
    close(ht_helper[next_id][read_fd]);
}

// Passing a message happens only if there is unit in a buffer
// and the semaphore of the next Helper can be decreased.
void Helper::processLoop() {
    receiveFromWorker();
    if (buffer.busy && sem_trywait(ready[next_id]) == 0) {
        pass_unit_to_next_helper();
    }
    receiveFromHelper();
    usleep(sleep_short);
}

// Check if Worker sent any message, if it does then write it to a Helper's
// buffer and if it is a message with a unit then set unit_type
void Helper::receiveFromWorker() {
    if (!read_unit(wt_helper[id][read_fd], buffer.unit)) { return; }
    logReceiveFromWorker(buffer.unit);
    usleep(sleep_long);
    buffer.busy = true;
    if (!buffer.unit.is_request) { unit_type = buffer.unit.type; }
}

// Helper sends a message to its Worker if:
// 1. Received message is a request and Worker is waiting for a request to
// produce.
// 2. If the type of received unit matches the type that Worker produced.
void Helper::handleReceivedMessage(message u) {
    if ((u.is_request && !unit_type.has_value()) || (u.type == unit_type)) {
        write_unit(ht_worker[id][write_fd], u);
        logSendToWorker(u);
        usleep(sleep_long);
        sem_post(ready[id]);
        unit_type.reset();
        return;
    }
    buffer.busy = true;
}

// Check if previous Helper sent any message from previous Helper.
void Helper::receiveFromHelper() {
    if (!read_unit(ht_helper[id][read_fd], buffer.unit)) { return; }
    logReceiveFromHelper(buffer.unit);
    handleReceivedMessage(buffer.unit);
}

// Passees unit to the next Helper.
void Helper::pass_unit_to_next_helper() {
    write_unit(ht_helper[next_id][write_fd], buffer.unit);
    logSendToHelper(buffer.unit);
    usleep(sleep_long);
    sem_post(ready[id]);
    buffer.busy = false;
}

// ---------- LOGGING ----------

void Helper::logSendToWorker(message u) const {
    if (u.is_request) {
        std::println("[Helper {}] Sent request to produce to its worker", id);
    } else {
        std::println(
            "[Helper {}] Sent unit {} produced by Worker {} to its worker",
            id,
            unit_type_to_string(u.type),
            u.producer_id);
    }
}

void Helper::logReceiveFromWorker(message u) const {
    if (u.is_request) {
        std::println("[Helper {}] Received request from worker", id);
    } else {
        std::println("[Helper {}] Received unit of type {} from worker",
                     id,
                     unit_type_to_string(u.type));
    }
}

void Helper::logReceiveFromHelper(message u) const {
    if (u.is_request) {
        std::println("[Helper {}] Received request to produce", id);
    } else {
        std::println(
            "[Helper {}] Received unit of type {} produced by Worker {}",
            id,
            unit_type_to_string(u.type),
            u.producer_id);
    }
}

void Helper::logSendToHelper(message u) const {
    if (u.is_request) {
        std::println("[Helper {}] Passed request to next Helper", id);
    } else {
        std::println("[Helper {}] Passed unit of type {} to next Helper",
                     id,
                     unit_type_to_string(u.type));
    }
}