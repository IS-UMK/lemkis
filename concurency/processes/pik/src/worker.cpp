#include "worker.h"

#include "pipes.h"
#include "prelude.h"
#include "semaphores.h"
#include "unit.h"

// In order to prevent a deadlock, at the start of the program
// only half of the Workers (those with odd ID number)
// can send units to its Helpers.
// The rest of them can only produce unit when they receive a signal to produce.
Worker::Worker(int id)
    : id(id), shouldSendSignal(id % number_two == 0), received{} {
    srand(time(nullptr) + getpid());
}

void Worker::run() {
    close_unused_pipes();
    while (true) { processLoop(); }
}

void Worker::close_unused_pipes() const {
    close(wt_helper[id][read_fd]);
    close(ht_worker[id][write_fd]);
}

// After receiving a unit from its Helper - Worker sends back a signal to
// produce for another Worker. After receiving a signal to produce from its
// Helper - Worker produces unit, sends it to helper and waits for a unit that
// has the same type as the one that it produced.
void Worker::processLoop() {
    if (!shouldSendSignal) { sendUnitToHelper(); }
    receiveFromHelper();
    if (shouldSendSignal) { sendSignalToHelper(); }
}

// Worker can send a message only if Helper's semaphores can be decreased.
// If it can't be that means Helper's buffer is full and Worker must wait for it
// to empty.
void Worker::sendUnitToHelper() {
    sem_wait(ready[id]);
    const message u = produce(id);
    write_unit(wt_helper[id][write_fd], u);
    logUnitSent();
    usleep(sleep_long);
}

void Worker::sendSignalToHelper() {
    sem_wait(ready[id]);
    const message signal{static_cast<UnitType>(0), id, true};
    write_unit(wt_helper[id][write_fd], signal);
    logSignalSent();
    usleep(sleep_long);
}

// When Worker receives message it changes it state
// to either sending a unit or sending a signal to produce.
void Worker::receiveFromHelper() {
    read_unit(ht_worker[id][read_fd], received);
    shouldSendSignal = handleReceivedMessage(received);
}

auto Worker::handleReceivedMessage(const message& u) -> bool {
    logMessageReceived(u);
    usleep(sleep_long);
    return !u.is_request;
}

// ---------- LOGGING ----------

void Worker::logUnitSent() const {
    std::println("[Worker {}] Sent unit to Helper", id);
}

void Worker::logSignalSent() const {
    std::println("[Worker {}] Sent Request signal", id);
}

void Worker::logMessageReceived(const message& u) const {
    if (u.is_request) {
        std::println("[Worker {}] Got request to produce", id);
    } else {
        std::println("[Worker {}] Received and consumed unit of type {}",
                     id,
                     unit_type_to_string(u.type));
    }
}
