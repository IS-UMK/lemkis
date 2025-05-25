#pragma once
#include <mutex>
#include <vector>

#include "Company.h"
#include "Employee.h"
#include "Printer.h"

/// <summary>
/// class which represents the office with multiple companies and printers,
/// which is ready for printer-chaos.
/// </summary>
class office {
    std::mutex mtx_;  // Mutex for synchronizing access to the office
    std::condition_variable cv_;  // Condition variable for signaling between
                                  // threads, when some printer is available

  public:
    std::vector<printer> printers;   // Vector of printers in the office
    std::vector<company> companies;  // Vector of companies in the office
    std::vector<std::thread>
        threads;  // Vector of threads representing employees

    auto run_chaos(int num_companies,
                   int num_printers,
                   int num_of_all_employees) -> void;

  private:
    void init_printers(int num_printers);
    void init_companies(int num_companies);
    void init_employees_threads(int num_companies, int num_of_all_employees);
};

/// <summary>
/// Method which represents the office with multiple companies and printers,
/// to simulate the chaos of why having multiple companies in one office is a
/// bad idea.
/// </summary>
/// <param name="num_companies"></param>
/// <param name="num_printers"></param>
/// <param name="num_of_all_employees"></param>
inline void office::run_chaos(const int num_companies,
                              const int num_printers,
                              const int num_of_all_employees) {
    init_printers(num_printers);    // init printers
    init_companies(num_companies);  // init companies
    init_employees_threads(num_companies,
                           num_of_all_employees);  // init employees threads

    for (auto& thread : threads)  // wait for threads to be done
    {
        thread.join();
    }
}

inline void office::init_printers(const int num_printers) {
    printers.reserve(num_printers);
    for (int i = 0; i < num_printers; i++) { printers.emplace_back(i); }
}

inline void office::init_companies(const int num_companies) {
    companies.reserve(num_companies);
    for (int i = 0; i < num_companies; i++) { companies.emplace_back(i); }
}

inline void office::init_employees_threads(const int num_companies,
                                           const int num_of_all_employees) {
    for (int i = 0; i < num_of_all_employees; i++) {
        const auto company_id = i % num_companies;
        std::printf("Creating employee %d from company %d\n", i, company_id);

        // run the employee thread with the company and printers
        threads.emplace_back(
            employee(i, companies[company_id], printers, mtx_, cv_));
    }
}
