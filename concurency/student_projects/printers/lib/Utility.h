#pragma once
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

    /// <summary>
    /// Generate a random integer in [min, max]
    /// </summary>
    static auto get_random_int(const int min, const int max) -> int {
        const std::lock_guard<std::mutex> lock(rng_mutex());
        std::uniform_int_distribution<> dist(min, max);
        return dist(generator());
    }

    /// <summary>
    /// Checks if the parameters for the office are valid.
    /// All values must be positive and there must be at least as many
    /// printers as companies
    /// </summary>
    auto static are_params_ok(const int num_companies,
                              const int num_printers,
                              const int num_of_all_employees) -> bool {
        return num_printers <= num_companies && num_printers > 0 &&
               num_companies > 0 && num_of_all_employees > 0;
    }

  private:
    /// <summary>
    /// Random number generator and mutex for thread-safe access
    /// </summary>
    static auto generator() -> std::mt19937& {
        static std::mt19937 gen(std::random_device{}());
        return gen;
    }

    static auto rng_mutex() -> std::mutex& {
        static std::mutex mtx;
        return mtx;
    }
};
