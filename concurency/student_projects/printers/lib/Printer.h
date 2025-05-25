#pragma once

#include <mutex>
#include <stdexcept>

#include "Utility.h"

/// <summary>
/// class which represents a printer in the office.
/// </summary>
class printer {
  public:
    explicit printer(const int id)
        : printer_id(id),
          current_company(utility::no_company),
          mtx(new std::mutex()) {
        // Ensure the printer ID is valid
        if (printer_id < 0) {
            throw std::invalid_argument("Printer ID cannot be negative");
        }
    }

    int printer_id;
    int current_company;
    int usage_count{0};

    std::mutex* mtx;  // each printer has its own mutex to ensure thread safety
};
