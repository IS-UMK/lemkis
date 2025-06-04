#pragma once

#include <chrono>

class timer {
  public:
    using clock = std::chrono::high_resolution_clock;
    using duration = std::chrono::milliseconds;

    auto start() -> void { m_start = clock::now(); }

    [[nodiscard]] auto elapsed() const -> duration {
        return std::chrono::duration_cast<duration>(clock::now() - m_start);
    }

  private:
    clock::time_point m_start{};
};