#include "Utility.h"

class mes_util {
  public:
    static void acquire_printer_print(const int company_id,
                                      const int printer_id) {
        utility::print(
            "Company {} acquired printer {}", company_id, printer_id);
    }

    static void employee_start_printing_print(const int company_id,
                                              const int employee_id,
                                              const int printer_id,
                                              const std::string& text) {
        utility::print(
            "Employee {} from company {} is printing on printer {} - with doc: "
            "{}",
            employee_id,
            company_id,
            printer_id,
            text);
    }

    static void employee_stop_printing_print(int company_id,
                                             int employee_id,
                                             int printer_id) {
        utility::print(
            "Employee {} from company {} has stopped printing on printer {}",
            employee_id,
            company_id,
            printer_id);
    }

    static void release_printer_print(const int company_id, int printer_id) {
        utility::print(
            "Company {} has released printer {}", company_id, printer_id);
    }

    static void employee_waiting_for_printer_print(int employee_id,
                                                   int company_id) {
        utility::print("Employee {} from company {} is waiting for a printer.",
                       employee_id,
                       company_id);
    }
    static void employee_got_notification_print(int employee_id,
                                                int company_id) {
        utility::print(
            "Employee {} from company {} got notification that a printer is "
            "available.",
            employee_id,
            company_id);
    }

    static void employee_trying_to_acquire_printer_print(int employee_id,
                                                         int company_id) {
        utility::print(
            "Employee {} from company {} is trying to acquire a printer.",
            employee_id,
            company_id);
    }

    static void employee_got_to_printer_print(int employee_id,
                                              int company_id,
                                              int printer_id) {
        utility::print("Employee {} from company {} got to printer with ID {}.",
                       employee_id,
                       company_id,
                       printer_id);
    }

    static void company_received_notification_from_employees(
        const int company_id) {
        utility::print(
            "Company {} received notification from employees to acquire a "
            "printer.",
            company_id);
    }

    static void company_could_not_acquire_printer_print(const int company_id) {
        utility::print(
            "Company {} could not acquire a printer, will wait for next "
            "notification.",
            company_id);
    }

    static void company_received_notification_from_office(
        const int company_id,
        const int assigned_printer) {
        utility::print(
            "Company {} received notification from office that a printer "
            "is available. Current company printer_id: {}",
            company_id,
            assigned_printer);
    }

    static void company_recv_notif_from_emps_to_release_printer(
        const int company_id) {
        utility::print(
            "Company {} received notification from employees to release a "
            "printer.",
            company_id);
    }

    static void employee_is_starting_print(const int employee_id,
                                           const int company_id,
                                           const __pid_t process_id) {
        utility::print("Employee {} from company {} with PID {} is starting.",
                       employee_id,
                       company_id,
                       process_id);
    }
};