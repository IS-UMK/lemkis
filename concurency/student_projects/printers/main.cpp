#include "Office.h"

auto constexpr num_companies = 782;
auto constexpr num_printers = 123;
auto constexpr num_of_all_employees = 4564;
auto constexpr invalid_params_exit_code = -2;
auto constexpr unexpected_exception_exit_code = 2;

/// <summary>
/// W budynku biurowym jest N firm. Jest również 0 < K ≤ N
/// drukarek, z których korzystają pracownicy tych firm. Zasady korzystania z
/// drukarek są następujące: Pracownicy różnych firm nigdy nie używają tej samej
/// drukarki jednocześnie (zakładamy, że istnieje procedura print(int
/// printer_number), której wywołanie oznacza korzystanie z drukarki: wysłanie
/// dokumentu, oczekiwanie na wydrukowanie i odebranie dokumentu z drukarki).
/// Pracownik danej firmy nie może rozpocząć drukowania, dopóki nie będzie
/// pewien, że drukarka, do której chce wysłać dokument, nie jest używana przez
/// pracowników innych firm. Pracownicy tej samej firmy mogą korzystać tylko z
/// jednej drukarki na raz. Jeśli jest wiele wolnych drukarek, pracownik,
/// którego firma aktualnie nie drukuje, może wybrać dowolną z nich (nie zależy
/// nam na równym rozłożeniu wykorzystania drukarek).
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
        if (!utility::are_params_ok(
                num_companies, num_printers, num_of_all_employees)) {
            return invalid_params_exit_code;
        }
        office my_office(num_companies, num_printers, num_of_all_employees);
        my_office.run_chaos();
        return 0;
    } catch (...) { return unexpected_exception_exit_code; }
}
