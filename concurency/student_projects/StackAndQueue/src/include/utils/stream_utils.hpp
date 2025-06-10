#pragma once

#include <cstddef>
#include <ostream>

inline auto to_streamsize(std::size_t size) -> std::streamsize {
    return static_cast<std::streamsize>(size);
}