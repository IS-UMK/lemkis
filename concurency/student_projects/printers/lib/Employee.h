#pragma once
#include <semaphore.h>

#include <cstdio>
#include <print>
#include <thread>

#include "Company.h"
#include "CompanyPack.h"
#include "Printer.h"
#include "Utility.h"


/// <summary>
/// class represents an employee in the office.
/// </summary>
class employee {
  public:
    auto static constexpr min_print_time = 300;
    auto static constexpr max_print_time = 900;
    auto static constexpr usage_value = 1;

    employee(int id,
             int company_id,
             company_pack* companies,
             printer* printers,
             sem_t* notify_from_company)
        : employee_id_(id),
          company_id_(company_id),
          companies_(companies),
          printers_(printers),
          notify_from_office_condition_(notify_from_company) {}

    void init_communication(sem_t* notify_company_acquire_condition,
                            sem_t* notify_company_release_condition) {
        notify_company_acquire_condition_ = notify_company_acquire_condition;
        notify_company_release_condition_ = notify_company_release_condition;
    }

    void run() {
        auto& printer = acquire();
        printing(printer);
        stop_printing(printer);
    }


  private:
    int employee_id_;
    int company_id_;
    company_pack* companies_;
    printer* printers_;

    sem_t* notify_from_office_condition_;
    sem_t* notify_company_acquire_condition_;
    sem_t* notify_company_release_condition_;

    auto get_company() -> company& {
        return companies_[company_id_].company_instance;
    }

    auto acquire() -> printer& {
        while (true) {
            mes_util::employee_trying_to_acquire_printer_print(employee_id_,
                                                               company_id_);
            if (!does_my_company_has_printer()) {
                handle_employee_waiting();
                continue;
            }
            return handle_employee_getting_to_printer();
        }
    }

    void handle_employee_waiting() {
        mes_util::employee_waiting_for_printer_print(employee_id_, company_id_);
        sem_post(notify_company_acquire_condition_);
        sem_wait(notify_from_office_condition_);
        mes_util::employee_got_notification_print(employee_id_, company_id_);
    }

    auto handle_employee_getting_to_printer() -> printer& {
        mes_util::employee_got_to_printer_print(
            employee_id_, company_id_, get_company().assigned_printer.load());
        auto& printer = printers_[get_company().assigned_printer];
        return printer;
    }


    auto does_my_company_has_printer() -> bool {
        auto& company = get_company();
        return company.assigned_printer.load() != utility::no_printer;
    }

    void printing(printer& printer) {
        mes_util::employee_start_printing_print(get_company().company_id,
                                                employee_id_,
                                                printer.printer_id,
                                                printer.generate_printer_doc());
        update_printer_usage(printer);
        simulate_work();
        mes_util::employee_stop_printing_print(
            get_company().company_id, employee_id_, printer.printer_id);
    }

    void stop_printing(printer& printer) {
        printer.usage_count.fetch_sub(usage_value);
        if (!printer::is_printer_free_usage_empty(printer)) { return; }
        utility::print("Employee {} wants to release printer with ID {}.",
                       employee_id_,
                       printer.printer_id);
        sem_post(notify_company_release_condition_);
        utility::print("Employee {} successfully sent release notify.",
                       employee_id_);
    }

    static void simulate_work() {
        const int print_time =
            utility::get_random_int(min_print_time, max_print_time);
        std::this_thread::sleep_for(std::chrono::milliseconds(print_time));
    }

    static void update_printer_usage(printer& printer) {
        printer.usage_count.fetch_add(usage_value);
        printer.times_used.fetch_add(usage_value);
    }
};