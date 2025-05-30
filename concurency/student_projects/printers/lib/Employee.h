#pragma once
#include <semaphore.h>

#include <cstdio>
#include <iostream>
#include <thread>

#include "Company.h"
#include "Printer.h"
#include "Utility.h"


/// <summary>
/// class represents an employee in the office.
/// </summary>
class employee {
  public:
    auto static constexpr min_print_time = 100;
    auto static constexpr max_print_time = 500;

    employee(int id,
             int company_id,
             company* companies,
             printer* printers,
             int num_printers,
             sem_t* mutex,
             sem_t* condition)
        : employee_id_(id),
          company_id_(company_id),
          companies_(companies),
          printers_(printers),
          num_printers_(num_printers),
          mutex_(mutex),
          condition_(condition) {}

    void run();

  private:
    int employee_id_;
    int company_id_;
    company* companies_;
    printer* printers_;
    int num_printers_;
    sem_t* mutex_;
    sem_t* condition_;

    auto get_company() -> company& { return companies_[company_id_]; }

    void acquire_printer_print(int printer_id) const {
        std::cout << "Company " << company_id_ << " acquired printer "
                  << printer_id << '\n'
                  << std::flush;
    }

    void employee_start_printing_print(int printer_id) const {
        std::cout << "Employee " << employee_id_ << " from company "
                  << company_id_ << " is printing on printer " << printer_id
                  << '\n'
                  << std::flush;
    }

    void employee_stop_printing_print(int printer_id) const {
        std::cout << "Employee " << employee_id_ << " from company "
                  << company_id_ << " has stopped printing on printer "
                  << printer_id << '\n'
                  << std::flush;
    }

    void release_printer_print(int printer_id) const {
        std::cout << "Company " << company_id_ << " has released printer "
                  << printer_id << '\n'
                  << std::flush;
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
        printer.usage_count++;
        printer.times_used++;
    }

    auto try_to_get_printer() -> bool;
    void get_printer(printer& printer);
    auto run_printer_checks(printer& printer) -> bool;
    void employee_starts_printing(printer& printer);
    void company_release_printer(printer& printer);
};

inline void employee::run() {
    acquire();
    const int printer_id = printing();
    release(printer_id);
}

inline void employee::acquire() {
    while (true) {
        sem_wait(mutex_);
        if (try_to_get_printer()) {
            sem_post(mutex_);
            break;
        }
        sem_post(mutex_);
        sem_wait(condition_);
    }
}

inline auto employee::try_to_get_printer() -> bool {
    if (get_company().assigned_printer != utility::no_printer) {
        employee_starts_printing(printers_[get_company().assigned_printer]);
        return true;
    }
    for (auto i = 0; i < num_printers_; i++) {
        if (run_printer_checks(printers_[i])) { return true; }
    }
    return false;
}

inline auto employee::run_printer_checks(printer& printer) -> bool {
    if (printer.current_company == company_id_) {
        employee_starts_printing(printer);
        return true;
    }
    if (printer.current_company == utility::no_company) {
        get_printer(printer);
        return true;
    }
    return false;
}

inline void employee::employee_starts_printing(printer& printer) {
    update_printer_usage(printer);
    employee_start_printing_print(printer.printer_id);
}

inline void employee::get_printer(printer& printer) {
    printer.current_company = company_id_;
    update_printer_usage(printer);
    get_company().assigned_printer = printer.printer_id;
    acquire_printer_print(printer.printer_id);
}

inline auto employee::printing() -> int {
    const int printer_id = get_company().assigned_printer;
    employee_start_printing_print(printer_id);
    simulate_work();
    return printer_id;
}

inline void employee::release(int printer_id) {
    sem_wait(mutex_);

    auto& printer = printers_[printer_id];
    printer.usage_count--;
    employee_stop_printing_print(printer_id);
    if (printer.usage_count == 0) { company_release_printer(printer); }

    sem_post(mutex_);
    sem_post(condition_);
}

inline void employee::company_release_printer(printer& printer) {
    printer.current_company = utility::no_company;
    get_company().assigned_printer = utility::no_printer;
    release_printer_print(printer.printer_id);
}
