#pragma once
#include <array>
#include <cerrno>
#include <climits>
#include <cstring>
#include <mutex>
#include <random>

/// <summary>
/// back-alley in the office, where all the utility functions are stored.
/// </summary>
class utility {
  public:
    static constexpr auto no_company = -1;
    static constexpr auto no_printer = -1;

    // Generate a random integer in [min, max]
    static auto get_random_int(const int min, const int max) -> int {
        const std::lock_guard<std::mutex> lock(rng_mutex());
        std::uniform_int_distribution<> dist(min, max);
        return dist(generator());
    }

  private:
    // Random number generator and mutex for thread-safe access
    static auto generator() -> std::mt19937& {
        static std::mt19937 gen(std::random_device{}());
        return gen;
    }

    static auto rng_mutex() -> std::mutex& {
        static std::mutex mtx;
        return mtx;
    }
};
