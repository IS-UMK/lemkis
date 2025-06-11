#pragma once

#include <semaphore.h>

#include "Company.h"

struct company_pack {

  private:
    static constexpr auto pshared_value = 1;  // shared between processes
    static constexpr auto shm_value = 0;      // initial value for semaphores
    static constexpr auto mutex_value = 1;    // initial value for mutex
  public:
    company company_instance;

    std::thread company_to_office_thread;

    sem_t company_mutex;
    sem_t notify_from_company_condition;
    sem_t notify_office_of_free_printer_condition;
    sem_t notify_my_employees_from_office_condition;
    sem_t notify_company_acquire_condition;
    sem_t notify_company_release_condition;

    void init_threads(company_pack* companies_pack, int num_companies) {
        company_to_office_thread =
            std::thread(&company_pack::run_office_helper_for_company,
                        this,
                        companies_pack,
                        num_companies);
    }

    void init_communication() {
        init_mutex();
        init_conditions();
        init_company_instance();
    }

    void init_mutex() { sem_init(&company_mutex, pshared_value, mutex_value); }

    void init_conditions() {
        sem_init(
            &notify_office_of_free_printer_condition, pshared_value, shm_value);
        sem_init(&notify_my_employees_from_office_condition,
                 pshared_value,
                 shm_value);
        sem_init(&notify_company_acquire_condition, pshared_value, shm_value);
        sem_init(&notify_company_release_condition, pshared_value, shm_value);
        sem_init(&notify_from_company_condition, pshared_value, shm_value);
    }

    void init_company_instance() {
        company_instance.init_mutex(&company_mutex);
        company_instance.init_employee_semaphores(
            &notify_my_employees_from_office_condition,
            &notify_company_acquire_condition,
            &notify_company_release_condition);
        company_instance.init_office_semaphores(
            &notify_office_of_free_printer_condition,
            &notify_from_company_condition);
    }

    void notify_first_waiting_printer(company_pack* companies_pack,
                                      int num_companies) {
        auto& my_company = company_instance;
        for (int i = 0; i < num_companies; i++) {
            if (i == my_company.company_id) {
                continue;  // Skip the current company
            }
            auto& comp = companies_pack[i].company_instance;
            if (handle_try_to_notify_printer(my_company, comp)) { return; }
        }
    }

  private:
    static auto handle_try_to_notify_printer(company& my_company, company& comp)
        -> bool {
        sem_wait(comp.mutex);
        if (company::is_company_waiting_for_printer(comp)) {
            my_company.notify_waiting_printer(comp);
            sem_post(comp.mutex);
            return true;
        }
        sem_post(comp.mutex);
        return false;
    }

    void run_office_helper_for_company(company_pack* company_pack,
                                       int num_companies) {
        while (true) {
            sem_wait(&notify_office_of_free_printer_condition);
            notify_first_waiting_printer(company_pack, num_companies);
            std::println("Company {} notified that a printer is available.",
                         company_instance.company_id);
        }
    }
};