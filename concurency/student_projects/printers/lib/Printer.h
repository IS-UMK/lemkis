#pragma once

#include <semaphore.h>

#include <atomic>
#include <format>
#include <string>

#include "Utility.h"


/// <summary>
/// class which represents a printer in the office.
/// </summary>
struct printer {
    int printer_id;
    std::atomic<int> current_company;
    std::atomic<int> usage_count;
    std::atomic<int> times_used;

    sem_t mutex;

    void init_data(int id) {
        printer_id = id;
        current_company.store(utility::no_company);
        usage_count.store(0);
        times_used.store(0);
    }

    void init_communication() {
        const int pshared = 1;  // shared between processes
        const int initial_value = 1;
        sem_init(&mutex, pshared, initial_value);
    }

    static auto is_printer_free_usage_empty(printer& p) -> bool {
        int no_usage = 0;
        auto result = p.usage_count.compare_exchange_strong(no_usage, 0);
        return result;
    }

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
