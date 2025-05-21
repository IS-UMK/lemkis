#pragma once

#include <array>
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <string_view>
#include <thread>
#include <vector>

#include "benchmark_params.hpp"

namespace csts {
    constexpr std::array<int, 3> thrc = {1, 2, 4};
    constexpr int one = 1;
    constexpr std::string_view mut = "mutex";
    constexpr std::string_view cv = "cv";
    constexpr std::string_view cex = "compare-exchange";
    constexpr std::string_view ato = "atomic";
    constexpr std::string_view ves = "vector_stack";
    constexpr std::string_view lss = "list_stack";
    constexpr std::string_view tsq = "two_stack_queue";
    constexpr std::string_view lfq = "lock_free_queue";
}  // namespace csts

using ThreadVec = std::vector<std::jthread>;

namespace benchmark {
    template <typename Func>
    auto measure_time(Func&& func) -> long long;

    auto print_res(BenchParams p, long long dur) -> void;

    template <typename T>
    auto st_mut_prod_work(T& st, std::atomic<int>& cnt, int items) -> void;

    template <typename T>
    auto st_cv_prod_work(T& st, std::atomic<int>& cnt, int items) -> void;

    template <typename T>
    auto make_st_mut_prods_helper(T& st, std::atomic<int>& cnt, int items)
        -> auto;

    template <typename T>
    auto make_st_cv_prods_helper(T& st, std::atomic<int>& cnt, int items)
        -> auto;

    template <typename T>
    auto create_st_mut_prods(T& st, ThreadVec& prods, int total, int num_prods)
        -> void;

    template <typename T>
    auto create_st_cv_prods(T& st, ThreadVec& prods, int total, int num_prods)
        -> void;

    template <typename T>
    auto process_st_mut_item(T& st,
                             std::atomic<int>& cnt,
                             std::atomic<bool>& done,
                             int total) -> bool;

    template <typename T>
    auto process_st_mut_items(T& st,
                              std::atomic<int>& cnt,
                              std::atomic<bool>& done,
                              int items_per,
                              int total) -> void;

    template <typename T>
    auto make_st_mut_cons_helper(T& st,
                                 std::atomic<int>& cnt,
                                 std::atomic<bool>& done,
                                 int items_per,
                                 int total) -> auto;

    template <typename T>
    auto create_st_mut_cons(T& st,
                            ThreadVec& cons,
                            int total,
                            int num_cons,
                            std::atomic<bool>& done) -> void;

    template <typename T>
    auto st_cv_cons_work(T& st, std::atomic<int>& cnt, int items) -> void;

    template <typename T>
    auto make_st_cv_cons_helper(T& st, std::atomic<int>& cnt, int items)
        -> auto;

    template <typename T>
    auto create_st_cv_cons(T& st, ThreadVec& cons, int total, int num_cons)
        -> void;

    template <typename T>
    auto create_st_prods(T& st,
                         ThreadVec& prods,
                         int total,
                         int num_prods,
                         std::string_view method) -> void;

    template <typename T>
    auto create_st_cons(T& st,
                        ThreadVec& cons,
                        int total,
                        int num_cons,
                        std::string_view method,
                        std::atomic<bool>& done) -> void;

    template <typename T>
    auto q_mut_prod_work(T& q, std::atomic<int>& cnt, int items) -> void;

    template <typename T>
    auto make_q_mut_prods_helper(T& q, std::atomic<int>& cnt, int items)
        -> auto;

    template <typename T>
    auto create_q_mut_prods(T& q, ThreadVec& prods, int total, int num_prods)
        -> void;

    template <typename T>
    auto q_cv_prod_work(T& q, std::atomic<int>& cnt, int items) -> void;

    template <typename T>
    auto make_q_cv_prods_helper(T& q, std::atomic<int>& cnt, int items) -> auto;

    template <typename T>
    auto create_q_cv_prods(T& q, ThreadVec& prods, int total, int num_prods)
        -> void;

    template <typename T>
    auto process_q_mut_item(T& q,
                            std::atomic<int>& cnt,
                            std::atomic<bool>& done,
                            int total) -> bool;

    template <typename T>
    auto process_q_mut_items(T& q,
                             std::atomic<int>& cnt,
                             std::atomic<bool>& done,
                             int items_per,
                             int total) -> void;

    template <typename T>
    auto make_q_mut_cons_helper(T& q,
                                std::atomic<int>& cnt,
                                std::atomic<bool>& done,
                                int items_per,
                                int total) -> auto;

    template <typename T>
    auto create_q_mut_cons(T& q,
                           ThreadVec& cons,
                           int total,
                           int num_cons,
                           std::atomic<bool>& done) -> void;

    template <typename T>
    auto q_cv_cons_work(T& q, std::atomic<int>& cnt, int items) -> void;

    template <typename T>
    auto make_q_cv_cons_helper(T& q, std::atomic<int>& cnt, int items) -> auto;

    template <typename T>
    auto create_q_cv_cons(T& q, ThreadVec& cons, int total, int num_cons)
        -> void;

    template <typename T>
    auto create_q_prods(T& q,
                        ThreadVec& prods,
                        int total,
                        int num_prods,
                        std::string_view method) -> void;

    template <typename T>
    auto create_q_cons(T& q,
                       ThreadVec& cons,
                       int total,
                       int num_cons,
                       std::atomic<bool>& done,
                       std::string_view method) -> void;

    template <typename T>
    auto lf_q_prod_work(T& q, std::atomic<int>& cnt, int items) -> void;

    template <typename T>
    auto make_lf_q_prods_helper(T& q, std::atomic<int>& cnt, int items) -> auto;

    template <typename T>
    auto create_lf_q_prods(T& q, ThreadVec& prods, int total, int num_prods)
        -> void;

    template <typename T>
    auto process_lf_q_item(T& q,
                           std::atomic<int>& cnt,
                           std::atomic<bool>& done,
                           int total) -> bool;

    template <typename T>
    auto process_lf_q_items(T& q,
                            std::atomic<int>& cnt,
                            std::atomic<bool>& done,
                            int items_per,
                            int total) -> void;

    template <typename T>
    auto make_lf_q_cons_helper(T& q,
                               std::atomic<int>& cnt,
                               std::atomic<bool>& done,
                               int items_per,
                               int total) -> auto;

    template <typename T>
    auto create_lf_q_cons(T& q,
                          ThreadVec& cons,
                          int total,
                          int num_cons,
                          std::atomic<bool>& done) -> void;

    auto wait_for_completion(ThreadVec& prods,
                             ThreadVec& cons,
                             std::atomic<bool>& done) -> void;

    template <typename T>
    auto setup_st_threads(T& st,
                          ThreadVec& prods,
                          ThreadVec& cons,
                          int total,
                          int n_prods,
                          int n_cons,
                          std::string_view method,
                          std::atomic<bool>& done) -> void;

    template <typename T>
    auto measure_st_time(BenchParams p) -> int;

    template <typename T>
    auto benchmark_st(std::string_view method, BenchParams p) -> void;

    template <typename T>
    auto setup_q_threads(T& q,
                         ThreadVec& prods,
                         ThreadVec& cons,
                         int total,
                         int n_prods,
                         int n_cons,
                         std::string_view method,
                         std::atomic<bool>& done) -> void;

    template <typename T>
    auto measure_q_time(BenchParams p) -> int;

    template <typename T>
    auto benchmark_q(BenchParams p) -> void;

    template <typename T>
    auto setup_lf_q_threads(T& q,
                            ThreadVec& prods,
                            ThreadVec& cons,
                            int total,
                            int n_prods,
                            int n_cons,
                            std::atomic<bool>& done) -> void;

    template <typename T>
    auto measure_lf_q_time(int n_prods, int n_cons, int total) -> int;

    template <typename T>
    auto benchmark_lf_q(BenchParams p) -> void;

    auto run_st_bench_for_type(BenchParams p) -> void;

    auto run_st_bench_inner(int prods, int cons, int total) -> void;

    auto run_stack_benchmarks(int total) -> void;

    auto run_q_bench_inner(int prods, int cons, int items) -> void;

    auto run_queue_benchmarks(int total) -> void;

    auto run_lf_q_concurrent(int items) -> void;

    auto run_lf_q_rw(int items) -> void;

    auto run_lock_free_queue_benchmarks(int total) -> void;

    auto run_new_benchmarks(int N) -> void;
}  // namespace benchmark