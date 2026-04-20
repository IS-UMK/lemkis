#pragma once

#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include <print>

#include "Company.h"
#include "CompanyPack.h"
#include "Employee.h"
#include "Printer.h"
#include "SharedData.h"
#include "Utility.h"


/// <summary>
/// class which represents the office with multiple companies and printers,
/// which is ready for printer-chaos.
/// </summary>
class office {
  public:
    office(int num_companies, int num_printers, int num_of_all_employees) {
        shared_data_ = new shared_data(num_companies, num_printers);
        num_of_all_employees_ = num_of_all_employees;
        num_companies_ = num_companies;
    }

    auto run_chaos() -> void;

  private:
    shared_data* shared_data_ = nullptr;
    int num_of_all_employees_;
    int num_companies_;


    static void prepare_printers(printer* printers, int num_printers) {
        for (int i = 0; i < num_printers; i++) {
            printers[i].init_data(i);
            printers[i].init_communication();
        }
    }

    static void prepare_companies(shared_data& sh,
                                  company_pack* company_packs,
                                  int num_companies) {
        for (int i = 0; i < num_companies; i++) {
            company_packs[i].company_instance.init_data(
                i, sh.printers_, sh.num_printers_);
            company_packs[i].init_communication();
        }
    }

    static void prepare_office(shared_data& shared_data) {
        auto& printers = shared_data.printers_;
        auto& company_packs = shared_data.companies_packs_;
        auto num_printers = shared_data.num_printers_;
        auto num_companies = shared_data.num_companies_;
        prepare_printers(printers, num_printers);
        prepare_companies(shared_data, company_packs, num_companies);
    }

    void run_companies();
    void run_employees();

    void run_employee_process(int employee_id);
    [[nodiscard]] auto create_employee(int employee_id,
                                       int company_id,
                                       company_pack& cp) const -> employee;

    void wait_for_employees() const {
        for (int i = 0; i < num_of_all_employees_; i++) { wait(nullptr); }
        utility::print("All employees finished.");
    }

    void wait_for_companies() const {
        for (int i = 0; i < num_companies_; i++) {
            auto& company_pack = shared_data_->companies_packs_[i];
            company_pack.company_instance.stop();
        }
        utility::print("All companies finished.");
    }

    void clean_up() const { delete shared_data_; }
    void print_summary() const;
};

inline void office::run_chaos() {
    prepare_office(*shared_data_);
    run_companies();
    run_employees();
    wait_for_employees();
    wait_for_companies();
    print_summary();
    clean_up();
}

inline void office::run_companies() {
    for (int i = 0; i < num_companies_; i++) {
        auto& company_pack = shared_data_->companies_packs_[i];
        company_pack.company_instance.start();
        company_pack.init_threads(shared_data_->companies_packs_,
                                  shared_data_->num_companies_);
    }
}


/// <summary>
/// Runs the employee processes.
/// </summary>
inline void office::run_employees() {
    for (auto i = 0; i < num_of_all_employees_; i++) {
        const pid_t pid = fork();
        if (pid == 0) {
            run_employee_process(i);
        } else {
            utility::print("Error creating employee {}", i);
        }
    }
}

inline void office::run_employee_process(const int employee_id) {
    auto process_id = getpid();
    auto company_id = employee_id % num_companies_;
    auto& pack = shared_data_->companies_packs_[company_id];
    mes_util::employee_is_starting_print(employee_id, company_id, process_id);
    auto emp = create_employee(employee_id, company_id, pack);
    emp.init_communication(&pack.notify_company_acquire_condition,
                           &pack.notify_company_release_condition);
    emp.run();
    exit(0);
}

inline auto office::create_employee(const int employee_id,
                                    const int company_id,
                                    company_pack& cp) const -> employee {
    employee emp(employee_id,
                 company_id,
                 shared_data_->companies_packs_,
                 shared_data_->printers_,
                 &cp.notify_my_employees_from_office_condition);
    return emp;
}

inline void office::print_summary() const {
    std::println("Printers usage summary:");
    std::fflush(stdout);
    for (int i = 0; i < shared_data_->num_printers_; i++) {
        std::println("Printer {} was used {} times.",
                     shared_data_->printers_[i].printer_id,
                     shared_data_->printers_[i].times_used.load());
        std::fflush(stdout);
    }
}