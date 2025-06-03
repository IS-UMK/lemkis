#include "Office.h"

auto constexpr num_companies = 782;
auto constexpr num_printers = 123;
auto constexpr num_of_all_employees = 4564;
auto constexpr invalid_params_exit_code = -2;
auto constexpr unexpected_exception_exit_code = 2;

auto main() noexcept -> int {
    try {
        if (!utility::are_params_ok(
                num_companies, num_printers, num_of_all_employees)) {
            return invalid_params_exit_code;
        }
        office my_office(num_companies, num_printers, num_of_all_employees);
        my_office.run_chaos();
        return 0;
    } catch (...) { return unexpected_exception_exit_code; }
}
