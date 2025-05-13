#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <array>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <map>
#include <print>
#include <queue>
#include <vector>

const int number_of_workers = 5;
// Three types of pipes
std::array<std::array<int, 2>, number_of_workers>
    wt_helper;  // Worker -> Helper
std::array<std::array<int, 2>, number_of_workers>
    ht_worker;  // Helper -> Worker
std::array<std::array<int, 2>, number_of_workers>
    ht_helper;  // Helper -> next Helper

using UnitType = int;  // Unit type definition

// Struct reprsenting unit produced by Workers
struct unit {
    UnitType type;    // Unit type (e.g. 0,1,2,...)
    int producer_id;  // ID of the Worker that created the unit
};


// Function that converts unit type to string (e.g. "0", "1", ...)
auto unit_type_to_string(UnitType type) { return std::to_string(type); }


// Produce units
auto produce(int id) -> unit {
    const unit u{static_cast<UnitType>(rand() % number_of_workers), id};
    std::println("[Worker {}] Produced unit of type {}",
                 id,
                 unit_type_to_string(u.type));
    return u;
}


// Function that read from the pipe
auto read_unit(int fd, unit &u) {
    const ssize_t bytes_read = read(fd, &u, sizeof(unit));
    return bytes_read == sizeof(unit);
}


// Function that write to the pipe
auto write_unit(int fd, const unit &u) {
    const ssize_t bytes_written = write(fd, &u, sizeof(unit));
    return bytes_written == sizeof(unit);
}


auto main() -> int {

    // Create pipes
    for (int i = 0; i < number_of_workers; ++i) {
        pipe(wt_helper[i].data());
        pipe(ht_worker[i].data());
        pipe(ht_helper[i].data());
    }

    // Create helpers
    for (int i = 0; i < number_of_workers; ++i) {
        const pid_t pid = fork();
        if (pid == 0) {
            const int id = i;
            const int next =
                (i + 1) % number_of_workers;  // ID of the next Helper
            unit buffer;                // stores the unit

            // Close unnecessary pipe ends
            close(wt_helper[id][1]);  // Helper only reads data from Worker
            close(ht_worker[id][0]);  // Helper only writes data to Worker
            close(ht_helper[id][1]);  // Helper only reads data from the
                                      // previous Helper
            close(ht_helper[next]
                           [0]);  // Helper only writes data to the next Helper
            while (true) {
                unit u;

                // // Read data from Worker
                read_unit(wt_helper[id][0], u);

                std::println(
                    "[Helper {}] Received unit of type {} from its worker",
                    id,
                    unit_type_to_string(u.type));

                buffer = u;

                // If received unit has the same type as a Helper/Worker than
                // end the while loop and send unit to Worker
                while (buffer.type != id) {
                    write_unit(ht_helper[next][1], buffer);
                    std::println(
                        "[Helper {}] Passed unit of type {} to Helper {}",
                        id,
                        unit_type_to_string(buffer.type),
                        next);

                    read_unit(ht_helper[id][0], u);
                    buffer = u;

                    std::println("[Helper {}] Received unit of type {} produced by Worker {}",
                        id,
                        unit_type_to_string(buffer.type),
                        buffer.producer_id);
                }

                // // Send unit to its Worker
                write_unit(ht_worker[id][1], buffer);
                std::println("[Helper {}] Sent unit of type {} produced by Worker {} to its worker ", id, unit_type_to_string(buffer.type), buffer.producer_id);
            }
            exit(0);
        }
    }

    // Create Workers
    for (int i = 0; i < number_of_workers; ++i) {
        const pid_t pid = fork();
        if (pid == 0) {
            const int id = i;

            close(wt_helper[id][0]);  // Worker only writes to Helper
            close(ht_worker[id][1]);  // Worker only reads from Helper
            srand(time(nullptr) +
                  getpid());  // Initialize random number generator

            while (true) {
                sleep(2);  // Use sleep to slow down production of units

                const unit u = produce(id);

                // Send unit to the appropriate Helper
                write_unit(wt_helper[id][1], u);
                std::println("[Worker {}] Sent unit to Helper {}", id, id);

                // Read unit from the pipe if it's accessible
                unit rcv;
                read_unit(ht_worker[id][0], rcv);

                std::println(
                    "[Worker {}] Received and consumed unit of type {} "
                    "produced by Worker {}",
                    id,
                    unit_type_to_string(rcv.type),
                    rcv.producer_id);
            }
            exit(0);
        }
    }

    // Main function waits for the end of all processes
    for (int i = 0; i < 2 * number_of_workers; ++i) { wait(nullptr); }

    return 0;
}
