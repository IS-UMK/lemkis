#ifndef PIPES_H
#define PIPES_H

#include <array>

#include "prelude.h"

extern std::array<std::array<int, pipes_ends_number>, number_of_workers>
    wt_helper;
extern std::array<std::array<int, pipes_ends_number>, number_of_workers>
    ht_worker;
extern std::array<std::array<int, pipes_ends_number>, number_of_workers>
    ht_helper;

void create_pipes();

void close_pipes_parent();

#endif
