#ifndef COMMON_H
#define COMMON_H

constexpr int number_of_workers = 5;
constexpr int sleep_short = 5000;
constexpr int sleep_long = 100000;
constexpr int chmod = 0666;
constexpr int sem_open_value = 1;
constexpr int number_of_all_processe = 2 * number_of_workers;
constexpr int pipes_ends_number = 2;
constexpr int number_two = 2;
constexpr int read_fd = 0;
constexpr int write_fd = 1;
constexpr int error_value_byte = -1;

#endif
