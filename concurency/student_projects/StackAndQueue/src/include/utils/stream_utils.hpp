/**
 * @file stream_utils.hpp
 * @brief Contains utility functions for stream operations.
 */

#pragma once

#include <cstddef>
#include <ostream>

/**
 * @brief Safely converts a std::size_t value to std::streamsize.
 *
 * This is used when writing binary or formatted data to output streams,
 * ensuring the size passed is of correct type.
 *
 * @param size The size value to convert.
 * @return The size as std::streamsize.
 */
inline auto to_streamsize(std::size_t size) -> std::streamsize {
    return static_cast<std::streamsize>(size);
}