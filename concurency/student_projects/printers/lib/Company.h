#pragma once
#include <condition_variable>
#include <vector>

#include "Printer.h"

/// <summary>
/// class represents a company in the office.
/// </summary>
class company {
  public:
    explicit company(const int id)
        : company_id(id), assigned_printer(utility::no_printer) {
        // Ensure the company ID is valid
        // if (company_id < 0) {
        //     throw std::invalid_argument("Company ID cannot be negative");
        // }
    }

    int company_id;
    int assigned_printer;

    void acquire_printer(std::vector<printer>& printers,
                         std::mutex& mtx,
                         std::condition_variable& cv);
    auto release_printer(std::vector<printer>& printers,
                         std::mutex& mtx,
                         std::condition_variable& cv) -> bool;

  private:
    auto can_use_assigned_printer(std::vector<printer>& printers) const -> bool;
    auto try_acquire_any_available_printer(std::vector<printer>& printers)
        -> bool;
    auto check_printer(printer& current_printer, size_t i) -> bool;
    auto try_get_printer(printer& printer, size_t i) -> bool;
    void acquire_printer(printer& printer, size_t i);
    void check_and_release_printer(printer& printer,
                                   std::condition_variable& cv);
    void release(printer& printer, std::condition_variable& cv);
};

/// <summary>
/// Method which acquires a printer for the company, which uses office's mutex
/// and condition variable to ensure synchronization between companies.
/// </summary>
/// <param name="printers"> all printers in office </param>
/// <param name="mtx"> companies mutex </param>
/// <param name="cv"> companies condition_variable </param>
inline void company::acquire_printer(std::vector<printer>& printers,
                                     std::mutex& mtx,
                                     std::condition_variable& cv) {
    std::unique_lock<std::mutex> lock(mtx);
    if (can_use_assigned_printer(printers)) { return; }
    while (true) {
        if (try_acquire_any_available_printer(printers)) { return; }
        cv.wait(lock);
    }
}

inline auto company::can_use_assigned_printer(
    std::vector<printer>& printers) const -> bool {
    if (assigned_printer == utility::no_printer) { return false; }

    printers[assigned_printer].usage_count++;
    return true;
}

inline auto company::try_acquire_any_available_printer(
    std::vector<printer>& printers) -> bool {
    for (size_t i = 0; i < printers.size(); i++) {
        auto& current_printer = printers.at(i);
        const std::unique_lock<std::mutex> printer_lock(*current_printer.mtx);

        if (check_printer(current_printer, i)) { return true; }
    }

    return false;
}

/// <summary>
/// if (assigned_printer == current_printer.printer_id) -> we found our
/// companies printer, while searching the office if (assigned_printer !=
/// utility::no_printer) { return false; } -> we need to find our companies
/// printer further
/// </summary>
/// <param name="current_printer"></param>
/// <param name="i"></param>
/// <returns></returns>
inline auto company::check_printer(printer& current_printer,
                                   const size_t i) -> bool {
    if (assigned_printer == current_printer.printer_id) {
        current_printer.usage_count++;
        return true;
    }
    if (assigned_printer != utility::no_printer) { return false; }
    if (try_get_printer(current_printer, i)) { return true; }
    return false;
}

inline auto company::try_get_printer(printer& printer, const size_t i) -> bool {
    if (printer.printer_id == assigned_printer) {
        printer.usage_count++;
        return true;
    }
    if (printer.current_company == utility::no_company) {
        acquire_printer(printer, i);
        return true;
    }
    return false;
}

inline void company::acquire_printer(printer& printer, const size_t i) {
    const auto int_i = static_cast<int>(i);
    printer.current_company = company_id;
    printer.usage_count++;
    assigned_printer = int_i;

    std::printf("Company %d acquired printer %d\n", company_id, int_i);
}

/// <summary>
/// Method which releases a printer for the company, which uses office's mutex
/// and condition variable to ensure synchronization between companies.
/// </summary>
/// <param name="printers"> all printers in office </param>
/// <param name="mtx"> companies mutex </param>
/// <param name="cv"> companies condition_variable </param>
inline auto company::release_printer(std::vector<printer>& printers,
                                     std::mutex& mtx,
                                     std::condition_variable& cv) -> bool {
    const std::unique_lock<std::mutex> lock(mtx);  // start lock
    const auto p =
        assigned_printer;  // getting printer_id from the assigned printer
    auto& printer = printers.at(p);
    check_and_release_printer(printer, cv);
    return printer.usage_count ==
           0;  // return true if printer is no longer in use
}

inline void company::check_and_release_printer(printer& printer,
                                               std::condition_variable& cv) {
    const std::unique_lock<std::mutex> printer_lock(
        *printer.mtx);      // lock printer for safe release
    printer.usage_count--;  // decrease usage count

    // notify if there is no usage, so other employees from other companies can
    // use it
    if (printer.usage_count == 0) {
        release(printer, cv);  // notify waiting companies
    }
}

inline void company::release(printer& printer, std::condition_variable& cv) {
    printer.current_company = utility::no_company;  // reset current company
    assigned_printer = utility::no_printer;         // reset assigned printer
    cv.notify_all();                                // notify waiting companies
}