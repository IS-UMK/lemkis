#pragma once

#include <atomic>
#include <thread>

namespace struct_test {
    auto run_queue_test() -> void;
    auto run_stack_test() -> void;
    template <typename SName>
    auto demonstrate_data_race(int item_count) -> void;
    template <typename SName>
    auto demonstrate_concurrent(const std::string& name, int item_count)
        -> void;
    template <typename SName>
    auto demonstrate_cv(int item_count) -> void;
    template <typename SName>
    auto dr_create_producer(SName& s,
                            int item_count,
                            std::atomic<int>& count,
                            std::atomic<bool>& race_flag) -> std::jthread;
    template <typename SName>
    auto dr_create_consumer(SName& s,
                            int item_count,
                            std::atomic<int>& count,
                            std::atomic<bool>& race_flag) -> std::jthread;
    auto dr_should_continue(int current,
                            int max,
                            const std::stop_token& stoken,
                            const std::atomic<bool>& race_flag) -> bool;
    auto dr_try_consume(auto& s,
                        std::atomic<int>& count,
                        std::atomic<bool>& race_flag) -> void;
    auto dr_wait(std::jthread& producer, std::jthread& consumer) -> void;
    auto dr_print_results(const std::atomic<int>& produced,
                          const std::atomic<int>& consumed,
                          bool race_detected) -> void;
    template <typename SName>
    auto conc_create_producer(SName& safe_struct,
                              int item_count,
                              std::atomic<int>& count) -> std::jthread;
    template <typename SName>
    auto conc_create_consumer(SName& safe_struct, std::atomic<int>& count)
        -> std::jthread;
    auto conc_wait(std::jthread& producer, std::jthread& consumer) -> void;
    template <typename SName>
    auto conc_print_results(const std::atomic<int>& produced,
                            const std::atomic<int>& consumed,
                            const SName& safe_struct) -> void;
    template <typename SName>
    auto cv_create_producer(SName& safe_struct,
                            int item_count,
                            std::atomic<int>& count) -> std::jthread;
    template <typename SName>
    auto cv_create_consumer(SName& safe_struct, std::atomic<int>& count)
        -> std::jthread;
    auto cv_wait(std::jthread& producer, std::jthread& consumer) -> void;
    auto cv_print_results(const std::atomic<int>& produced,
                          const std::atomic<int>& consumed) -> void;
}  // namespace struct_test