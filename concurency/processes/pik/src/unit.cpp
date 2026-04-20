#include "unit.h"

#include "prelude.h"

auto produce(int id) -> message {
    message u{static_cast<UnitType>(rand() % (unit_interval) + number_one),
              id,
              false};
    std::println("[Worker {}] Produced unit of type {}",
                 id,
                 unit_type_to_string(u.type));
    usleep(sleep_long);
    return u;
}

auto read_unit(int fd, message &u) -> bool {
    const ssize_t bytes_read = read(fd, &u, sizeof(message));
    if (bytes_read == error_value_byte) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) { return false; }
        perror("read");
        return false;
    }
    return bytes_read == sizeof(message);
}

auto write_unit(int fd, const message &u) -> bool {
    const ssize_t bytes_written = write(fd, &u, sizeof(message));
    if (bytes_written == error_value_byte) {
        perror("write");
        return false;
    }
    return bytes_written == sizeof(message);
}
