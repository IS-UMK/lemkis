#pragma once

#include <semaphore.h>

#include <atomic>
#include <format>
#include <string>


/// <summary>
/// class which represents a printer in the office.
/// </summary>
struct printer {
    int printer_id;
    std::atomic<int> current_company;
    std::atomic<int> usage_count{0};
    std::atomic<int> times_used{0};

    sem_t mutex;

    [[nodiscard]] auto generate_printer_doc() const -> std::string {
        auto current_company_id = current_company.load();
        auto usage_count_value = usage_count.load();
        auto times_used_value = times_used.load();
        return std::format(
            "Current Company: {}, Usage Count: {}, Times Used: {}",
            current_company_id,
            usage_count_value,
            times_used_value);
    }
};
