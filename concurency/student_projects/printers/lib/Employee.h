#pragma once
#include <semaphore.h>

#include <cstdio>
#include <print>
#include <thread>

#include "Company.h"
#include "Printer.h"
#include "Utility.h"


/// <summary>
/// class represents an employee in the office.
/// </summary>
class employee {
  public:
    auto static constexpr min_print_time = 200;
    auto static constexpr max_print_time = 500;
    auto static constexpr usage_value = 1;

    employee(int id,
             int company_id,
             company* companies,
             printer* printers,
             int num_printers,
             // sem_t* mutex,
             sem_t* condition)
        : employee_id_(id),
          company_id_(company_id),
          companies_(companies),
          printers_(printers),
          num_printers_(num_printers),
          // mutex_(mutex),
          condition_(condition) {}

    void run();

  private:
    int employee_id_;
    int company_id_;
    company* companies_;
    printer* printers_;
    int num_printers_;
    // sem_t* mutex_;
    sem_t* condition_;

    auto get_company() -> company& { return companies_[company_id_]; }

    void acquire_printer_print(int printer_id) const {
        std::println("Company {} acquired printer {}", company_id_, printer_id);
        std::fflush(stdout);
    }

    void employee_start_printing_print(int printer_id) const {
        auto text = printers_[printer_id].generate_printer_doc();
        std::println(
            "Employee {} from company {} is printing on printer {} - with doc: "
            "{}",
            employee_id_,
            company_id_,
            printer_id,
            text);
        std::fflush(stdout);
    }

    void employee_stop_printing_print(int printer_id) const {
        std::println(
            "Employee {} from company {} has stopped printing on printer {}",
            employee_id_,
            company_id_,
            printer_id);
        std::fflush(stdout);
    }

    void release_printer_print(int printer_id) const {
        std::println(
            "Company {} has released printer {}", company_id_, printer_id);
        std::fflush(stdout);
    }

    void acquire();
    auto printing() -> int;
    void release(int printer_id);

    static void simulate_work() {
        const int print_time =
            utility::get_random_int(min_print_time, max_print_time);
        std::this_thread::sleep_for(std::chrono::milliseconds(print_time));
    }

    static void update_printer_usage(printer& printer) {
        printer.usage_count.fetch_add(usage_value);
        printer.times_used.fetch_add(usage_value);
    }

    auto try_to_get_printer() -> bool;
    void get_printer(printer& printer);
    auto run_printer_checks(printer& printer) -> bool;
    void company_release_printer(printer& printer);
    void handle_printer_release(printer& printer);
};

inline void employee::run() {
    acquire();
    const int printer_id = printing();
    release(printer_id);
}

/// <summary>
/// Acquires a printer for the employee.
/// It will wait until a printer is available.
/// </summary>
inline void employee::acquire() {
    while (true) {
        if (try_to_get_printer()) { break; }
        sem_wait(condition_);
    }
}


/// <summary>
/// Tries to get a printer for the employee.
/// It first checks if the company has an assigned printer.
/// If not, it checks all printers.
/// </summary>
inline auto employee::try_to_get_printer() -> bool {
    if (get_company().assigned_printer != utility::no_printer) {
        auto& printer = printers_[get_company().assigned_printer];
        update_printer_usage(printer);
        return true;
    }
    for (auto i = 0; i < num_printers_; i++) {
        if (run_printer_checks(printers_[i])) { return true; }
    }
    return false;
}

/// <summary>
/// Runs checks on the printer to determine if the employee can use it.
/// </summary>
inline auto employee::run_printer_checks(printer& printer) -> bool {
    sem_wait(&printer.mutex);
    if (printer.current_company == utility::no_company) {
        get_printer(printer);
        sem_post(&printer.mutex);
        return true;
    }
    sem_post(&printer.mutex);
    return false;
}

/// <summary>
/// Employee gets the printer and assigns it to the company.
/// </summary>
inline void employee::get_printer(printer& printer) {
    printer.current_company = company_id_;
    update_printer_usage(printer);
    get_company().assigned_printer = printer.printer_id;
    acquire_printer_print(printer.printer_id);
}

/// <summary>
/// Employee starts printing on the assigned printer.
/// </summary>
inline auto employee::printing() -> int {
    const int printer_id = get_company().assigned_printer;
    employee_start_printing_print(printer_id);
    simulate_work();
    return printer_id;
}


/// <summary>
/// Employee releases the printer after printing. If printer is free it realeses
/// it and notifies others that it is available.
/// </summary>
inline void employee::release(int printer_id) {
    auto& printer = printers_[printer_id];
    // sem_wait(&printer.mutex);
    printer.usage_count.fetch_sub(usage_value);
    employee_stop_printing_print(printer_id);
    handle_printer_release(printer);
    // sem_post(&printer.mutex);
    sem_post(condition_);
}

inline void employee::handle_printer_release(printer& printer) {
    int expected = 0;
    if (printer.usage_count.compare_exchange_strong(expected, 0)) {
        company_release_printer(printer);
    }
}

/// <summary>
/// Company releases the printer and sets the assigned printer to no printer.
/// </summary>
inline void employee::company_release_printer(printer& printer) {
    sem_wait(&printer.mutex);
    printer.current_company = utility::no_company;
    get_company().assigned_printer = utility::no_printer;
    release_printer_print(printer.printer_id);
    sem_post(&printer.mutex);
}
