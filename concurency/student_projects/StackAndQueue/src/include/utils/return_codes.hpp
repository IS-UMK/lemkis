/**
 * @file return_codes.hpp
 * @brief Defines return codes used by the benchmark program.
 */

#pragma once

#include <cstdint>

/**
 * @enum return_codes
 * @brief Represents status codes returned from the main application.
 */
enum class return_codes : std::uint8_t {
    success = 0,  ///< Indicates successful execution.
    error = 1     ///< Indicates an error during execution.
};