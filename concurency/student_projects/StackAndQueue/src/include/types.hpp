#pragma once

#include <string_view>

using ThreadVec = std::vector<std::jthread>;

struct BenchParams {
    std::string_view name;
    int np;
    int nc;
    int total;
};

struct References {
    ThreadVec cons;
    ThreadVec prods;
    atomic<bool> done;
};