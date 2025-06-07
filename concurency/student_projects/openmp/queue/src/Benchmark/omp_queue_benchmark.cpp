
#include <omp.h>

#include <benchmark_utils.hpp>
#include <omp_queue.hpp>

constexpr int item_limit = 1000000;
constexpr int id_offset = 10000;
constexpr int k_increment = 1;

namespace {
    omp_queue<int> queue;
    int pushed = 0;
    int popped = 0;
    auto omp_producer(int tid, int per) -> void {
        for (int i = 0; i < per; ++i) {
            queue.push(i + (tid * id_offset));
#pragma omp atomic
            pushed += k_increment;
        }
    }

    auto atomic_pop_increment() {
#pragma omp atomic
        popped += k_increment;
    }

    auto try_pop(int total) -> bool {
        int current;
#pragma omp atomic read
        current = popped;
        if (current >= total) { return false; }
        if (queue.pop()) {
            atomic_pop_increment();
            return true;
        }
        return false;
    }

    auto consumer_action(int total) -> bool {
        if (!try_pop(total)) {
#pragma omp taskyield
            int cur;
#pragma omp atomic read
            cur = popped;
            return cur >= total;
        }
        return false;
    }

    auto omp_consumer_loop(int total) -> void {
        while (true) {
            if (consumer_action(total)) { break; }
        }
    }

    auto run_omp_threads(int prod,
                         [[maybe_unused]] int cons,
                         int per,
                         int total) -> void {
#pragma omp parallel num_threads(prod + cons)
        {
            int const tid = omp_get_thread_num();
            if (tid < prod) {
                omp_producer(tid, per);
            } else {
                omp_consumer_loop(total);
            }
        }
    }
}  // namespace
namespace omp_bench {
    auto omp_queue_test(int producers, int consumers)
        -> std::tuple<int, int, std::size_t> {
        int const per = item_limit / producers;
        int const total = per * producers;
        run_omp_threads(producers, consumers, per, total);
        return {pushed, popped, queue.size()};
    }
}  // namespace omp_bench