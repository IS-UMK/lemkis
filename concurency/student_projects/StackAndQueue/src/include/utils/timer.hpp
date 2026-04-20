/**
 * @file timer.hpp
 * @brief Provides a utility class for measuring elapsed time in milliseconds.
 */

#pragma once

#include <chrono>

/**
 * @class timer
 * @brief High-resolution timer for measuring durations in milliseconds.
 *
 * The timer can be started explicitly using start() and queried using
 * elapsed().
 */
class timer {
  public:
    using clock = std::chrono::high_resolution_clock;
    using duration = std::chrono::milliseconds;

    /**
     * @brief Starts or restarts the timer.
     */
    auto start() -> void { m_start = clock::now(); }

    /**
     * @brief Returns the duration in milliseconds since the last call to
     * start().
     * @return Duration elapsed in milliseconds.
     */
    [[nodiscard]] auto elapsed() const -> duration {
        return std::chrono::duration_cast<duration>(clock::now() - m_start);
    }

  private:
    clock::time_point m_start;
};