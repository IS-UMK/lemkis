#include <iostream>

#include "Office.h"

auto constexpr num_companies = 3;
auto constexpr num_printers = 2;
auto constexpr num_of_all_employees = 6;
auto constexpr invalid_params_exit_code = -2;
auto constexpr unexpected_exception_exit_code = 2;

auto are_params_ok() -> bool {
    // All values must be positive and there must be at least as many printers
    // as companies
    return num_printers <= num_companies && num_printers > 0 &&
           num_companies > 0 && num_of_all_employees > 0;
}

/// <summary>
/// W office znajduje sie $num_companies firm oraz dostepne jest $num_printers
/// drukarek, ktore sa wspoldzielone przez pracownikow tych firm.
///
/// Zasady:
/// 1. Pracownicy roznych firm nie moga korzystac z tej samej drukarki w tym
/// samym czasie.
///    Oznacza to ze jezeli firma 0 korzysta z drukarki 0, to firma 1 nie moze
///    korzystac z tej drukarki w tym samym czasie. Firma 1 bedzie mogla
///    korzystac z tej drukarki dopiero po tym jak firma 0 ja zwolni --->
///    usage_count == 0.
///
/// 2. Pracownik danej firmy nie moze rozpoczac drukowania, dopoki nie ma
/// pewnosci, ze wybrana drukarka jest przydzielona
///    do firmy w ktorej pracuje --- nie jest uzywana przez innych pracownikow
///    innej firmy.
///
/// 3. Pracownicy tej samej firmy moga korzystac z tej samej drukarki w tym
/// samym czasie.
///
/// 4. Jesli jest wiele wolnych drukarek, to pracownik moze wybrac dowolna z
/// nich, O ILE jego firma nie korzysta juz z innej drukarki.
/// </summary>
auto main() noexcept -> int {
    try {
        if (!are_params_ok()) { return invalid_params_exit_code; }
        office my_office;
        my_office.run_chaos(num_companies, num_printers, num_of_all_employees);
        return 0;
    } catch (...) { return unexpected_exception_exit_code; }
}
