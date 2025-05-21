#pragma once

#include "include/benchmark.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <print>
#include <string_view>
#include <thread>
#include <vector>

#include "include/list_stack.hpp"
#include "include/two_stack_queue.hpp"
#include "include/types.hpp"
#include "include/vector_stack.hpp"
#include "libs/concurrentqueue.h"
#include "libs/readerwriterqueue.h"

using HighResClock = std::chrono::high_resolution_clock;
using MillisecDuration = std::chrono::milliseconds;
using ThreadVec = std::vector<std::jthread>;

template <typename Func>
auto benchmark::measure_time(Func&& func) -> long long {
    auto start = HighResClock::now();
    func();
    auto end = HighResClock::now();
    return std::chrono::duration_cast<MillisecDuration>(end - start).count();
}

auto benchmark::print_res(BenchParams p, long long dur) -> void {
    std::print("{}: {} prods, {} cons, {} items - {} ms\n",
               p.name,
               p.np,
               p.nc,
               p.total,
               dur);
}

template <typename T>
auto benchmark::st_mut_prod_work(T& st, std::atomic<int>& cnt, int items)
    -> void {
    for (int i = 0; i < items; ++i) {
        st.mutex_push(i);
        cnt.fetch_add(1);
    }
}

template <typename T>
auto benchmark::st_cv_prod_work(T& st, std::atomic<int>& cnt, int items)
    -> void {
    for (int i = 0; i < items; ++i) {
        st.cv_push(i);
        cnt.fetch_add(1);
    }
}

template <typename T>
auto benchmark::make_st_mut_prods_helper(T& st,
                                         std::atomic<int>& cnt,
                                         int items) -> auto {
    return [&st, &cnt, items]() { st_mut_prod_work(st, cnt, items); };
}

template <typename T>
auto benchmark::make_st_cv_prods_helper(T& st, std::atomic<int>& cnt, int items)
    -> auto {
    return [&st, &cnt, items]() { st_cv_prod_work(st, cnt, items); };
}

template <typename T>
auto benchmark::create_st_mut_prods(T& st,
                                    ThreadVec& prods,
                                    int total,
                                    int num_prods) -> void {
    std::atomic<int> cnt(0);
    int items_per = total / num_prods;
    for (int p = 0; p < num_prods; ++p) {
        prods.emplace_back(make_st_mut_prods_helper(st, cnt, items_per));
    }
}

template <typename T>
auto benchmark::create_st_cv_prods(T& st,
                                   ThreadVec& prods,
                                   int total,
                                   int num_prods) -> void {
    std::atomic<int> cnt(0);
    int items_per = total / num_prods;
    for (int p = 0; p < num_prods; ++p) {
        prods.emplace_back(make_st_cv_prods_helper(st, cnt, items_per));
    }
}

template <typename T>
auto benchmark::process_st_mut_item(T& st,
                                    std::atomic<int>& cnt,
                                    std::atomic<bool>& done,
                                    int total) -> bool {
    auto item = st.mutex_pop();
    if (item) {
        cnt.fetch_add(1);
        return true;
    }
    if (done && cnt.load() >= total) { return false; }
    std::this_thread::yield();
    return true;
}

template <typename T>
auto benchmark::process_st_mut_items(T& st,
                                     std::atomic<int>& cnt,
                                     std::atomic<bool>& done,
                                     int items_per,
                                     int total) -> void {
    int processed = 0;
    while (processed < items_per) {
        if (!process_st_mut_item(st, cnt, done, total)) break;
        processed++;
    }
}

template <typename T>
auto benchmark::make_st_mut_cons_helper(T& st,
                                        std::atomic<int>& cnt,
                                        std::atomic<bool>& done,
                                        int items_per,
                                        int total) -> auto {
    return [&st, &cnt, &done, items_per, total]() {
        process_st_mut_items(st, cnt, done, items_per, total);
    };
}

template <typename T>
auto benchmark::create_st_mut_cons(T& st,
                                   ThreadVec& cons,
                                   int total,
                                   int num_cons,
                                   std::atomic<bool>& done) -> void {
    std::atomic<int> cnt(0);
    int items_per = total / num_cons;
    for (int c = 0; c < num_cons; ++c) {
        cons.emplace_back(
            make_st_mut_cons_helper(st, cnt, done, items_per, total));
    }
}

template <typename T>
auto benchmark::st_cv_cons_work(T& st, std::atomic<int>& cnt, int items)
    -> void {
    for (int i = 0; i < items; ++i) {
        st.cv_pop_wait();
        cnt.fetch_add(1);
    }
}

template <typename T>
auto benchmark::make_st_cv_cons_helper(T& st, std::atomic<int>& cnt, int items)
    -> auto {
    return [&st, &cnt, items]() { st_cv_cons_work(st, cnt, items); };
}

template <typename T>
auto benchmark::create_st_cv_cons(T& st,
                                  ThreadVec& cons,
                                  int total,
                                  int num_cons) -> void {
    std::atomic<int> cnt(0);
    int items_per = total / num_cons;
    for (int c = 0; c < num_cons; ++c) {
        cons.emplace_back(make_st_cv_cons_helper(st, cnt, items_per));
    }
}

template <typename T>
auto benchmark::create_st_prods(T& st,
                                ThreadVec& prods,
                                int total,
                                int num_prods,
                                std::string_view method) -> void {
    if (method == csts::mut) {
        create_st_mut_prods(st, prods, total, num_prods);
    } else if (method == csts::cv) {
        create_st_cv_prods(st, prods, total, num_prods);
    }
}

template <typename T>
auto benchmark::create_st_cons(T& st,
                               ThreadVec& cons,
                               int total,
                               int num_cons,
                               std::string_view method,
                               std::atomic<bool>& done) -> void {
    if (method == csts::mut) {
        create_st_mut_cons(st, cons, total, num_cons, done);
    } else if (method == csts::cv) {
        create_st_cv_cons(st, cons, total, num_cons);
    }
}

template <typename T>
auto benchmark::q_mut_prod_work(T& q, std::atomic<int>& cnt, int items)
    -> void {
    for (int i = 0; i < items; ++i) {
        q.mutex_enqueue(i);
        cnt.fetch_add(1);
    }
}

template <typename T>
auto benchmark::make_q_mut_prods_helper(T& q, std::atomic<int>& cnt, int items)
    -> auto {
    return [&q, &cnt, items]() { q_mut_prod_work(q, cnt, items); };
}

template <typename T>
auto benchmark::create_q_mut_prods(T& q,
                                   ThreadVec& prods,
                                   int total,
                                   int num_prods) -> void {
    std::atomic<int> cnt(0);
    int items_per = total / num_prods;
    for (int p = 0; p < num_prods; ++p) {
        prods.emplace_back(make_q_mut_prods_helper(q, cnt, items_per));
    }
}

template <typename T>
auto benchmark::q_cv_prod_work(T& q, std::atomic<int>& cnt, int items) -> void {
    for (int i = 0; i < items; ++i) {
        q.cv_enqueue(i);
        cnt.fetch_add(1);
    }
}

template <typename T>
auto benchmark::make_q_cv_prods_helper(T& q, std::atomic<int>& cnt, int items)
    -> auto {
    return [&q, &cnt, items]() { q_cv_prod_work(q, cnt, items); };
}

template <typename T>
auto benchmark::create_q_cv_prods(T& q,
                                  ThreadVec& prods,
                                  int total,
                                  int num_prods) -> void {
    std::atomic<int> cnt(0);
    int items_per = total / num_prods;
    for (int p = 0; p < num_prods; ++p) {
        prods.emplace_back(make_q_cv_prods_helper(q, cnt, items_per));
    }
}

template <typename T>
auto benchmark::process_q_mut_item(T& q,
                                   std::atomic<int>& cnt,
                                   std::atomic<bool>& done,
                                   int total) -> bool {
    auto item = q.mutex_dequeue();
    if (item) {
        cnt.fetch_add(1);
        return true;
    }
    if (done && cnt.load() >= total) { return false; }
    std::this_thread::yield();
    return true;
}

template <typename T>
auto benchmark::process_q_mut_items(T& q,
                                    std::atomic<int>& cnt,
                                    std::atomic<bool>& done,
                                    int items_per,
                                    int total) -> void {
    int processed = 0;
    while (processed < items_per) {
        if (!process_q_mut_item(q, cnt, done, total)) break;
        processed++;
    }
}

template <typename T>
auto benchmark::make_q_mut_cons_helper(T& q,
                                       std::atomic<int>& cnt,
                                       std::atomic<bool>& done,
                                       int items_per,
                                       int total) -> auto {
    return [&q, &cnt, &done, items_per, total]() {
        process_q_mut_items(q, cnt, done, items_per, total);
    };
}

template <typename T>
auto benchmark::create_q_mut_cons(T& q,
                                  ThreadVec& cons,
                                  int total,
                                  int num_cons,
                                  std::atomic<bool>& done) -> void {
    std::atomic<int> cnt(0);
    int items_per = total / num_cons;
    for (int c = 0; c < num_cons; ++c) {
        cons.emplace_back(
            make_q_mut_cons_helper(q, cnt, done, items_per, total));
    }
}

template <typename T>
auto benchmark::q_cv_cons_work(T& q, std::atomic<int>& cnt, int items) -> void {
    for (int i = 0; i < items; ++i) {
        q.cv_dequeue_wait();
        cnt.fetch_add(1);
    }
}

template <typename T>
auto benchmark::make_q_cv_cons_helper(T& q, std::atomic<int>& cnt, int items)
    -> auto {
    return [&q, &cnt, items]() { q_cv_cons_work(q, cnt, items); };
}

template <typename T>
auto benchmark::create_q_cv_cons(T& q, ThreadVec& cons, int total, int num_cons)
    -> void {
    std::atomic<int> cnt(0);
    int items_per = total / num_cons;
    for (int c = 0; c < num_cons; ++c) {
        cons.emplace_back(make_q_cv_cons_helper(q, cnt, items_per));
    }
}

template <typename T>
auto benchmark::create_q_prods(T& q,
                               ThreadVec& prods,
                               int total,
                               int num_prods,
                               std::string_view method) -> void {
    if (method == csts::mut) {
        create_q_mut_prods(q, prods, total, num_prods);
    } else if (method == csts::cv) {
        create_q_cv_prods(q, prods, total, num_prods);
    }
}

template <typename T>
auto benchmark::create_q_cons(T& q,
                              ThreadVec& cons,
                              int total,
                              int num_cons,
                              std::atomic<bool>& done,
                              std::string_view method) -> void {
    if (method == csts::mut) {
        create_q_mut_cons(q, cons, total, num_cons, done);
    } else if (method == csts::cv) {
        create_q_cv_cons(q, cons, total, num_cons);
    }
}

template <typename T>
auto benchmark::lf_q_prod_work(T& q, std::atomic<int>& cnt, int items) -> void {
    for (int i = 0; i < items; ++i) {
        q.enqueue(i);
        cnt.fetch_add(1);
    }
}

template <typename T>
auto benchmark::make_lf_q_prods_helper(T& q, std::atomic<int>& cnt, int items)
    -> auto {
    return [&q, &cnt, items]() { lf_q_prod_work(q, cnt, items); };
}

template <typename T>
auto benchmark::create_lf_q_prods(T& q,
                                  ThreadVec& prods,
                                  int total,
                                  int num_prods) -> void {
    std::atomic<int> cnt(0);
    int items_per = total / num_prods;
    for (int p = 0; p < num_prods; ++p) {
        prods.emplace_back(make_lf_q_prods_helper(q, cnt, items_per));
    }
}

template <typename T>
auto benchmark::process_lf_q_item(T& q,
                                  std::atomic<int>& cnt,
                                  std::atomic<bool>& done,
                                  int total) -> bool {
    int item;
    if (q.try_dequeue(item)) {
        cnt.fetch_add(1);
        return true;
    }
    if (done && cnt.load() >= total) { return false; }
    std::this_thread::yield();
    return true;
}

template <typename T>
auto benchmark::process_lf_q_items(T& q,
                                   std::atomic<int>& cnt,
                                   std::atomic<bool>& done,
                                   int items_per,
                                   int total) -> void {
    int processed = 0;
    while (processed < items_per) {
        if (!process_lf_q_item(q, cnt, done, total)) break;
        processed++;
    }
}

template <typename T>
auto benchmark::make_lf_q_cons_helper(T& q,
                                      std::atomic<int>& cnt,
                                      std::atomic<bool>& done,
                                      int items_per,
                                      int total) -> auto {
    return [&q, &cnt, &done, items_per, total]() {
        process_lf_q_items(q, cnt, done, items_per, total);
    };
}

template <typename T>
auto benchmark::create_lf_q_cons(T& q,
                                 ThreadVec& cons,
                                 int total,
                                 int num_cons,
                                 std::atomic<bool>& done) -> void {
    std::atomic<int> cnt(0);
    int items_per = total / num_cons;
    for (int c = 0; c < num_cons; ++c) {
        cons.emplace_back(
            make_lf_q_cons_helper(q, cnt, done, items_per, total));
    }
}

auto benchmark::wait_for_completion(ThreadVec& prods,
                                    ThreadVec& cons,
                                    std::atomic<bool>& done) -> void {
    for (auto& prod : prods) { prod.join(); }
    done = true;
    for (auto& con : cons) { con.join(); }
}

template <typename T>
auto benchmark::setup_st_threads(T& st,
                                 ThreadVec& prods,
                                 ThreadVec& cons,
                                 int total,
                                 int n_prods,
                                 int n_cons,
                                 std::string_view method,
                                 std::atomic<bool>& done) -> void {
    create_st_prods(st, prods, total, n_prods, method);
    create_st_cons(st, cons, total, n_cons, method, done);
}

template <typename T>
auto benchmark::measure_st_time(BenchParams p) -> int {
    T st;
    std::atomic<bool> done = false;
    ThreadVec prods, cons;
    prods.reserve(p.np);
    cons.reserve(p.nc);
    return measure_time([&] {
        setup_st_threads(st, prods, cons, total, n_prods, n_cons, method, done);
        wait_for_completion(prods, cons, done);
    });
}

template <typename T>
auto benchmark::benchmark_st(std::string_view method, BenchParams p) -> void {
    BenchParams p1{method, p.np, p.nc, p.total};
    int duration = measure_st_time<T>(p1);
    std::string_view full_name = std::format("{} with {}", name, method);
    p.name = full_name;
    print_res(p, duration);
}

template <typename T>
auto benchmark::setup_q_threads(T& q,
                                ThreadVec& prods,
                                ThreadVec& cons,
                                int total,
                                int n_prods,
                                int n_cons,
                                std::string_view method,
                                std::atomic<bool>& done) -> void {
    create_q_prods(q, prods, total, n_prods, method);
    create_q_cons(q, cons, total, n_cons, done, method);
}

template <typename T>
auto benchmark::measure_q_time(BenchParams p) -> int {
    T q;
    std::atomic<bool> done = false;
    ThreadVec prods, cons;
    prods.reserve(p.np);
    cons.reserve(p.nc);
    return measure_time([&] {
        setup_q_threads(q, prods, cons, total, n_prods, n_cons, method, done);
        wait_for_completion(prods, cons, done);
    });
}

template <typename T>
auto benchmark::benchmark_q(BenchParams p) -> void {
    int duration = measure_q_time<T>(p);
    std::string full_name = std::format("{} with {}", csts::tsq, p.name);
    p.name = full_name;
    print_res(p, duration);
}

template <typename T>
auto benchmark::setup_lf_q_threads(T& q, BenchParams p, References& r) -> void {
    create_lf_q_prods(q, prods, p.total, p.np);
    create_lf_q_cons(q, cons, total, p.nc, done);
}

template <typename T>
auto benchmark::measure_lf_q_time(BenchParams p) -> int {
    T q;
    References r;
    r.prods.reserve(n_prods);
    r.cons.reserve(n_cons);
    std::atomic<bool> done = false;
    return measure_time([&] {
        setup_lf_q_threads(q, p, r);
        wait_for_completion(r);
    });
}

template <typename T>
auto benchmark::benchmark_lf_q(BenchParams p) -> void {
    int duration = measure_lf_q_time<T>(p);
    std::string full_name = std::format("{} with {}", csts::lfq, p.name);
    p.name = full_name;
    print_res(param_type, duration);
}

auto benchmark::run_st_bench_for_type(BenchParams p) -> void {
    if (p.name == csts::ves) {
        benchmark_st<vector_stack<int>>(csts::mut, p);
        benchmark_st<vector_stack<int>>(csts::cv, p);
    } else {
        benchmark_st<list_stack<int>>(csts::mut, p);
        benchmark_st<list_stack<int>>(csts::cv, p);
    }
}

auto benchmark::run_st_bench_inner(int prods, int cons, int total) -> void {
    BenchParams p1{csts::ves, prods, cons, total};
    BenchParams p2{csts::lss, prods, cons, total};
    run_st_bench_for_type(p1);
    run_st_bench_for_type(p2);
}

auto benchmark::run_stack_benchmarks(int total) -> void {
    for (int const prods : csts::thrc) {
        for (int const cons : csts::thrc) {
            run_st_bench_inner(prods, cons, total);
        }
    }
}

auto benchmark::run_q_bench_inner(int prods, int cons, int items) -> void {
    BenchParams p1{csts::mut, prods, cons, items};
    benchmark_q<two_stack_queue<int>>(p1);
    BenchParams p2{csts::cv, prods, cons, items};
    benchmark_q<two_stack_queue<int>>(p2);
}

auto benchmark::run_queue_benchmarks(int total) -> void {
    for (int const prods : csts::thrc) {
        for (int const cons : csts::thrc) {
            run_q_bench_inner(prods, cons, total);
        }
    }
}

auto benchmark::run_lf_q_concurrent(int items) -> void {
    for (int const prods : csts::thrc) {
        for (int const cons : csts::thrc) {
            BenchParams p{csts::cex, prods, cons, items};
            benchmark_lf_q<moodycamel::ConcurrentQueue<int>>(p);
        }
    }
}

auto benchmark::run_lf_q_rw(int items) -> void {
    BenchParams p{csts::ato, csts::one, csts::one, items};
    benchmark_lf_q<moodycamel::ReaderWriterQueue<int>>(p);
}

auto benchmark::run_lock_free_queue_benchmarks(int total) -> void {
    run_lf_q_concurrent(total);
    run_lf_q_rw(total);
}

auto benchmark::run_new_benchmarks(int N) -> void {
    std::print("Running Stack Benchmarks:\n========================\n\n");
    run_stack_benchmarks(N);
    std::print("\nRunning Queue Benchmarks:\n========================\n\n");
    run_queue_benchmarks(N);
    std::print("\nRunning Lock-Free Queue Benchmarks:\n=================\n\n");
    run_lock_free_queue_benchmarks(N);
}