#pragma once

#include <thread>

#include "Company.h"

/// <summary>
/// class represents an employee in the office.
/// </summary>
class employee {
  public:
    auto static constexpr min_print_time = 100;
    auto static constexpr max_print_time = 500;

    employee(const int id,
             company& comp,
             std::vector<printer>& printers,
             std::mutex& mtx,
             std::condition_variable& cv)
        : employee_id_(id),
          company_(&comp),
          printers_(&printers),
          mtx_(&mtx),
          cv_(&cv) {}

    // auto run method to simulate the employee's work with the printer
    auto operator()() const -> void {
        acquire();
        const auto printer_id = printing();
        release(printer_id);
    }

    void acquire() const {
        company_->acquire_printer(
            *printers_,
            *mtx_,
            *cv_);  // try to acquire a printer for the employee's company
    }

    [[nodiscard]] auto printing() const -> int {
        const auto p = company_->assigned_printer;
        full_simulate_printing(employee_id_, company_->company_id, p);

        return p;  // return the assigned printer ID
    }

    void release(const int printer_id) const {
        const auto is_printer_empty = company_->release_printer(
            *printers_,
            *mtx_,
            *cv_);  // we are done, so we can release the printer
        release_info(is_printer_empty, printer_id);  // print release info
    }

    void release_info(const bool is_printer_empty, const int printer_id) const {
        std::printf("Employee %d from company %d released printer %d\n",
                    employee_id_,
                    company_->company_id,
                    printer_id);
        if (is_printer_empty) {
            std::printf("Company %d released printer %d\n",
                        company_->company_id,
                        printer_id);
        }
    }

  private:
    int employee_id_;
    company* company_;  // [cppcoreguidelines-avoid-const-or-ref-data-members]
    std::vector<printer>* printers_;
    std::mutex* mtx_;
    std::condition_variable* cv_;


    static void full_simulate_printing(int employee_id,
                                       int company_id,
                                       int assigned_printer);
    static void printing_simulation();
};

inline void employee::full_simulate_printing(const int employee_id,
                                             const int company_id,
                                             const int assigned_printer) {
    std::printf("Employee %d from company %d started using printer %d\n",
                employee_id,
                company_id,
                assigned_printer);
    printing_simulation();  // simulate printing time
    std::printf("Employee %d from company %d finished using printer %d\n",
                employee_id,
                company_id,
                assigned_printer);
}

inline void employee::printing_simulation() {
    const auto printing_time = std::chrono::milliseconds(
        utility::get_random_int(min_print_time, max_print_time));
    std::this_thread::sleep_for(printing_time);  // printing simulation
}
