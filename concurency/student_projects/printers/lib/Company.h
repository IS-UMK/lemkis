#pragma once

#include <semaphore.h>

#include <atomic>
#include <chrono>
#include <cstdio>
#include <print>
#include <thread>

#include "Messages.h"
#include "Printer.h"
#include "Utility.h"


/// <summary>
/// class represents a company in the office.
/// </summary>
class company {
  private:
    static auto constexpr shm_value = 1;
    static auto constexpr pshared_value = 1;

  public:
    int company_id;
    std::atomic<bool> is_on_cooldown;
    std::atomic<bool> should_wait_for_printer;
    std::atomic<int> assigned_printer;

    sem_t* mutex;
    sem_t* notify_from_company_condition;
    sem_t* notify_from_my_employees_acquire_condition;
    sem_t* notify_from_my_employees_release_condition;
    sem_t* notify_office_of_free_printer_condition;
    sem_t* notify_my_employees_from_office_condition;

    std::thread companies_communication_thread;
    std::thread employees_printer_waiting_thread;
    std::thread employees_release_waiting_thread;

    company() = default;

    void init_data(int id, printer* printers, int num_printers) {
        company_id = id;
        printers_ = printers;
        num_printers_ = num_printers;
        should_wait_for_printer.store(true);
        is_on_cooldown.store(false);
        assigned_printer.store(utility::no_printer);
    }

    void init_mutex(sem_t* company_mutex) { this->mutex = company_mutex; }

    void init_employee_semaphores(
        sem_t* notify_my_employees_from_office_condition,
        sem_t* notify_from_my_employees_acquire_condition,
        sem_t* notify_from_my_employees_release_condition) {
        this->notify_my_employees_from_office_condition =
            notify_my_employees_from_office_condition;
        this->notify_from_my_employees_acquire_condition =
            notify_from_my_employees_acquire_condition;
        this->notify_from_my_employees_release_condition =
            notify_from_my_employees_release_condition;
    }

    void init_office_semaphores(sem_t* notify_office_of_free_printer_condition,
                                sem_t* notify_company_acquire_condition) {
        this->notify_office_of_free_printer_condition =
            notify_office_of_free_printer_condition;
        this->notify_from_company_condition = notify_company_acquire_condition;
    }

    void run_companies_communication() {
        while (should_wait_for_printer.load()) {
            if (try_to_acquire_printer()) { notify_my_current_employees(); }
            sem_wait(notify_from_company_condition);
            mes_util::company_received_notification_from_office(
                company_id, assigned_printer.load());
        }
    }

    void run_employees_communication() {
        while (should_wait_for_printer.load()) {
            sem_wait(notify_from_my_employees_acquire_condition);
            mes_util::company_received_notification_from_employees(company_id);
            if (try_to_acquire_printer()) {
                notify_my_current_employees();
                continue;
            }
            mes_util::company_could_not_acquire_printer_print(company_id);
        }
    }

    void run_employees_release_communication() {
        while (should_wait_for_printer.load()) {
            sem_wait(notify_from_my_employees_release_condition);
            mes_util::company_received_notification_from_employees(company_id);
            release_printer();
            notify_my_current_employees();
        }
    }

    void start() {
        companies_communication_thread =
            std::thread(&company::run_companies_communication, this);
        employees_printer_waiting_thread =
            std::thread(&company::run_employees_communication, this);
        employees_release_waiting_thread =
            std::thread(&company::run_employees_release_communication, this);
        utility::print("Company {} started.", company_id);
    }

    void stop() {
        should_wait_for_printer.store(false);
        sem_post(notify_from_company_condition);
        sem_post(notify_from_my_employees_acquire_condition);
        companies_communication_thread.join();
        employees_printer_waiting_thread.join();
    }

    static auto is_company_waiting_for_printer(company& company) -> bool {
        auto expected = utility::no_printer;
        return company.assigned_printer.compare_exchange_strong(
                   expected, utility::no_printer) &&
               !company.is_on_cooldown.load();
    }

    void notify_waiting_printer(company& company) {
        sem_post(company.notify_from_company_condition);
        utility::print(
            "Company {} notified company {} that a printer is available.",
            company_id,
            company.company_id);
    }

  private:
    printer* printers_;
    int num_printers_;

    auto try_to_acquire_printer() -> bool {
        sem_wait(mutex);
        if (is_on_cooldown.load()) {
            sem_post(mutex);
            return false;
        }
        auto result = handle_acquire();
        sem_post(mutex);
        return result;
    }

    auto handle_acquire() -> bool {
        for (auto i = 0; i < num_printers_; i++) {
            auto& printer = printers_[i];
            sem_wait(&printer.mutex);
            auto result = handle_printer_and_try_get(printer);
            sem_post(&printer.mutex);
            if (result) { return true; }
        }
        return false;
    }

    auto handle_printer_and_try_get(printer& printer) -> bool {
        if (!is_printer_available(printer)) { return false; }
        acquire_printer(printer);
        return true;
    }

    static auto is_printer_available(printer& printer) -> bool {
        return printer.current_company.load() == utility::no_company;
    }

    void acquire_printer(printer& printer) {
        printer.current_company.store(company_id);
        assigned_printer.store(printer.printer_id);
        utility::print(
            "Company {} acquired printer {}.", company_id, printer.printer_id);
        sem_post(notify_my_employees_from_office_condition);
    }

    void release_printer() {
        auto& printer = get_company_printer();
        sem_wait(&printer.mutex);
        handle_printer_release(printer);
        sem_post(&printer.mutex);
        sem_post(notify_office_of_free_printer_condition);
        utility::print(
            "Company {} sent notification to office that printer is free.",
            company_id);
        go_on_cooldown();
    }

    void go_on_cooldown() {
        is_on_cooldown.store(true);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(utility::communication_delay));
        is_on_cooldown.store(false);
    }

    void handle_printer_release(printer& printer) {
        // if (printer.current_company.load() != company_id) { return; }
        printer.current_company.store(utility::no_company);
        assigned_printer.store(utility::no_printer);
        std::println(
            "Company {} released printer {}.", company_id, printer.printer_id);
        std::fflush(stdout);
    }

    auto get_company_printer() -> printer& {
        return printers_[assigned_printer.load()];
    }

    void notify_my_current_employees() const {
        sem_post(notify_my_employees_from_office_condition);
    }
};