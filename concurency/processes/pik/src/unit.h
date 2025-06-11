#ifndef UNIT_H
#define UNIT_H

#include <string>

using UnitType = int;

struct message {
    UnitType type;
    int producer_id;
    bool is_request = false;
};

inline auto unit_type_to_string(UnitType type) -> std::string {
    return std::to_string(type);
}

auto produce(int id) -> message;

auto read_unit(int fd, message &u) -> bool;

auto write_unit(int fd, const message &u) -> bool;

#endif
