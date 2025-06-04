#pragma once

#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#include <iostream>
#include <utility>

#include "Company.h"
#include "Employee.h"
#include "Printer.h"
#include "Utility.h"


/// <summary>
/// class which represents the office with multiple companies and printers,
/// which is ready for printer-chaos.
/// </summary>
class office {
  public:
    office(int num_companies, int num_printers, int num_of_all_employees)
        : num_companies_(num_companies),
          num_printers_(num_printers),
          num_of_all_employees_(num_of_all_employees) {}

    auto run_chaos() -> void;
    void run_employee_process(int i, sem_t* mutex, sem_t* condition) const;

  private:
    const int shm_mode = 0666;  // file permissions for shared memory
    const int shm_value = 1;    // initial value for semaphores
    const char* shm_printers_name = "/printers_shared_memory";
    const char* shm_companies_name = "/companies_shared_memory";

  public:
    int num_companies_;
    int num_printers_;
    int num_of_all_employees_;

    printer* printers_;   // shared memory for printers
    company* companies_;  // shared memory for companies

    static void prepare_printers(printer* printers, int num_printers) {
        for (int i = 0; i < num_printers; i++) {
            printers[i].printer_id = i;
            printers[i].current_company = utility::no_company;
            printers[i].usage_count = 0;
        }
    }

    static void prepare_companies(company* companies, int num_companies) {
        for (int i = 0; i < num_companies; i++) {
            companies[i].company_id = i;
            companies[i].assigned_printer = utility::no_printer;
        }
    }

    static void prepare_office(printer* printers,
                               int num_printers,
                               company* companies,
                               int num_companies) {
        prepare_printers(printers, num_printers);
        prepare_companies(companies, num_companies);
    }

    auto create_employee(const int i, sem_t* mutex, sem_t* condition) const
        -> employee {
        auto company_id = i % num_companies_;
        employee emp(i,
                     company_id,
                     companies_,
                     printers_,
                     num_printers_,
                     mutex,
                     condition);
        return emp;
    }

    void wait_for_employees() const {
        for (int i = 0; i < num_of_all_employees_; i++) { wait(nullptr); }
    }

    void clean_up() {
        printers_ = nullptr;
        companies_ = nullptr;
        sem_unlink("/mutex");
        sem_unlink("/condition");
        shm_unlink(shm_printers_name);
        shm_unlink(shm_companies_name);
    }

    void print_summary() const;
    void run_employees(sem_t* mutex, sem_t* condition) const;

    [[nodiscard]] auto stage_init_semaphores() const
        -> std::pair<sem_t*, sem_t*>;

    auto init_printers_shm(int num_printers) -> printer*;
    auto init_companies_shm(int num_companies) -> company*;
    void init_shared_memory();
};

inline void office::run_chaos() {
    clean_up();
    init_shared_memory();
    prepare_office(printers_, num_printers_, companies_, num_companies_);
    auto [mutex, condition] = stage_init_semaphores();
    run_employees(mutex, condition);
    wait_for_employees();
    print_summary();
    clean_up();
}

/// <summary>
/// Initializes shared memory for printers and companies.
/// </summary>
inline void office::init_shared_memory() {
    printers_ = init_printers_shm(num_printers_);
    companies_ = init_companies_shm(num_companies_);
}

/// <summary>
/// Initializes semaphores for synchronization.
/// </summary>
inline auto office::stage_init_semaphores() const -> std::pair<sem_t*, sem_t*> {
    sem_t* mutex = sem_open("/mutex", O_CREAT, shm_mode, shm_value);
    sem_t* condition = sem_open("/condition", O_CREAT, shm_mode, 0);
    return {mutex, condition};
}

/// <summary>
/// Runs the employee processes.
/// </summary>
inline void office::run_employees(sem_t* mutex, sem_t* condition) const {
    for (auto i = 0; i < num_of_all_employees_; i++) {
        const pid_t pid = fork();
        if (pid == 0) { run_employee_process(i, mutex, condition); }
    }
}

inline void office::run_employee_process(int i,
                                         sem_t* mutex,
                                         sem_t* condition) const {
    auto process_id = getpid();
    std::println("Employee {} with PID {} is starting.", i, process_id);
    auto emp = create_employee(i, mutex, condition);
    emp.run();
    exit(0);
}

/// <summary>
/// Initializes shared memory for printers.
/// </summary>
inline auto office::init_printers_shm(const int num_printers) -> printer* {
    const size_t printer_size = sizeof(printer) * num_printers;
    const int shm_fd = shm_open(shm_printers_name, O_CREAT | O_RDWR, shm_mode);
    auto length = static_cast<long>(printer_size);
    ftruncate(shm_fd, length);

    auto* printers = static_cast<printer*>(
        mmap(nullptr, length, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    return printers;
}

/// <summary>
/// Initializes shared memory for companies.
/// </summary>
inline auto office::init_companies_shm(const int num_companies) -> company* {
    const size_t company_size = sizeof(company) * num_companies;
    const int shm_fd = shm_open(shm_companies_name, O_CREAT | O_RDWR, shm_mode);
    auto length = static_cast<long>(company_size);
    ftruncate(shm_fd, length);

    auto* companies = static_cast<company*>(
        mmap(nullptr, length, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0));
    return companies;
}

inline void office::print_summary() const {
    std::cout << "Printers usage summary:\n";
    for (int i = 0; i < num_printers_; i++) {
        std::println("Printer {} was used {} times.",
                     printers_[i].printer_id,
                     printers_[i].times_used);
    }
}