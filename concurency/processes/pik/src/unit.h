#ifndef UNIT_H
#define UNIT_H

#include <string>

using UnitType = int;

struct unit {
    UnitType type;
    int producer_id;
    bool busy = false;
    bool is_request = false;
};

inline auto unit_type_to_string(UnitType type) -> std::string {
    return std::to_string(type);
}

auto produce(int id) -> unit;

auto read_unit(int fd, unit &u) -> bool;

auto write_unit(int fd, const unit &u) -> bool;

#endif
