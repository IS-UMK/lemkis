#pragma once

#include <iostream>
#include <format>
#include <string_view>

// Prevent actual inclusion of <print> if someone tries
#ifdef _LIBCPP_PRINT
#error "Do not include both <print> and print_compat.hpp"
#endif

// namespace std {
//     // Basic print implementation
//     template<typename... Args>
//     void print(const std::string_view fmt_str, Args&&... args) {
//         std::cout << std::format(fmt_str, std::forward<Args>(args)...);
//     }
    
//     // println implementation (with newline)
//     template<typename... Args>
//     void println(const std::string_view fmt_str, Args&&... args) {
//         std::cout << std::format(fmt_str, std::forward<Args>(args)...) << std::endl;
//     }
    
//     // // Special case for just printing a string without format args
//     // inline void print(const std::string_view str) {
//     //     std::cout << str;
//     // }
    
//     // inline void println(const std::string_view str) {
//     //     std::cout << str << std::endl;
//     // }
    
//     // // File versions (just write to stdout for compatibility)
//     // template<typename... Args>
//     // void print(FILE*, const std::string_view fmt_str, Args&&... args) {
//     //     std::cout << std::format(fmt_str, std::forward<Args>(args)...);
//     // }
    
//     // template<typename... Args>
//     // void println(FILE*, const std::string_view fmt_str, Args&&... args) {
//     //     std::cout << std::format(fmt_str, std::forward<Args>(args)...) << std::endl;
//     // }
// }