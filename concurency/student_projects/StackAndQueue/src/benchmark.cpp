#include "include/benchmark.hpp"

#include <atomic>
#include <chrono>
#include <mutex>
#include <print>
#include <thread>
#include <vector>

#include "include/con_queue.hpp"
#include "include/con_stack.hpp"
#include "include/list_stack.hpp"
#include "include/queue.hpp"
#include "include/stack.hpp"
#include "include/two_stack_queue.hpp"
#include "include/vector_stack.hpp"
#include "libs/concurrentqueue.h"
#include "libs/readerwriterqueue.h"

namespace benchmark {
// Constants to replace magic numbers
namespace constants {
constexpr int THREAD_COUNTS[] = {1, 2, 4};
constexpr int SINGLE_THREAD = 1;
constexpr char MUTEX_METHOD[] = "mutex";
constexpr char CV_METHOD[] = "cv";
constexpr char COMPARE_EXCHANGE[] = "compare-exchange";
constexpr char ATOMIC_METHOD[] = "atomic";
} // namespace constants

using HighResClock = std::chrono::high_resolution_clock;
using MillisecDuration = std::chrono::milliseconds;

template <typename StructType>
static void run_producers(StructType& s, std::atomic<int>& produced_count, int n_producers, int total_items, std::vector<std::jthread>& producers) {
   for (int p = 0; p < n_producers; ++p) {
      producers.emplace_back([&](const std::stop_token& stoken) {
         while (produced_count < total_items && !stoken.stop_requested()) {
            s.push(produced_count);
            produced_count++;
         }
      });
   }
}

template <typename StructType>
static void run_consumers(StructType& s, std::atomic<int>& consumed_count, int n_consumers, int total_items, std::vector<std::jthread>& consumers) {
   for (int p = 0; p < n_consumers; ++p) {
      consumers.emplace_back([&](const std::stop_token& stoken) {
         while (consumed_count < total_items && !stoken.stop_requested()) {
            if (!s.empty()) {
               s.pop();
               consumed_count++;
            }
         }
      });
   }
}

static void join_threads(auto& threads) {
   for (auto& thread : threads) {
      thread.join();
   }
}

static void print_results(const std::string& struct_name, int n_producers, int n_consumers, int total_items, auto duration) {
   std::print("{}: {} producers, {} consumers, {} items total - completed in {} ms\n", struct_name, n_producers, n_consumers, total_items, duration.count());
}

template <typename StructType>
static void setup_concurrent_benchmark(StructType& s, std::atomic<int>& produced_count, std::atomic<int>& consumed_count, std::vector<std::jthread>& producers, std::vector<std::jthread>& consumers, int n_producers, int n_consumers, int total_items) {
   run_producers(s, produced_count, n_producers, total_items, producers);
   run_consumers(s, consumed_count, n_consumers, total_items, consumers);
}

template <typename StructType>
static auto measure_execution_time(StructType& s, std::atomic<int>& produced_count, std::atomic<int>& consumed_count, std::vector<std::jthread>& producers, std::vector<std::jthread>& consumers, int n_producers, int n_consumers, int total_items) {
   auto start_time = HighResClock::now();
   setup_concurrent_benchmark(s, produced_count, consumed_count, producers, consumers, n_producers, n_consumers, total_items);
   join_threads(producers);
   join_threads(consumers);
   auto end_time = HighResClock::now();
   return std::chrono::duration_cast<MillisecDuration>(end_time - start_time);
}

template <typename StructType>
static void benchmark_old_concurrent_structure(const std::string& struct_name, int n_producers, int n_consumers, int total_items) {
   StructType s;
   std::atomic<int> produced_count(0);
   std::atomic<int> consumed_count(0);
   std::vector<std::jthread> producers;
   std::vector<std::jthread> consumers;
   auto duration = measure_execution_time(s, produced_count, consumed_count, producers, consumers, n_producers, n_consumers, total_items);
   print_results(struct_name, n_producers, n_consumers, total_items, duration);
}

template <typename StructType>
void run_old_structure_operations(StructType& s, std::atomic<int>& push_count, std::atomic<int>& pop_count, int total_items) {
   while (push_count < total_items) {
      s.push(push_count);
      push_count++;
   }
   while (pop_count < total_items) {
      s.pop();
      pop_count++;
   }
}

template <typename StructType>
auto benchmark_old_structure(const std::string& struct_name, int n_producers, int n_consumers, int total_items) {
   StructType s;
   std::atomic<int> push_count(0);
   std::atomic<int> pop_count(0);
   auto start_time = HighResClock::now();
   run_old_structure_operations(s, push_count, pop_count, total_items);
   auto end_time = HighResClock::now();
   auto duration = std::chrono::duration_cast<MillisecDuration>(end_time - start_time);
   print_results(struct_name, n_producers, n_consumers, total_items, duration);
}

void benchmark_old_concurrent_queues(int item_count) {
   for (int const p : constants::THREAD_COUNTS) {
      for (int const c : constants::THREAD_COUNTS) {
         benchmark_old_concurrent_structure<concurrent_queue<int>>(
             "Old concurrent queue", p, c, item_count);
      }
   }
}

void benchmark_old_concurrent_stacks(int item_count) {
   for (int const p : constants::THREAD_COUNTS) {
      for (int const c : constants::THREAD_COUNTS) {
         benchmark_old_concurrent_structure<concurrent_stack<int>>(
             "Old concurrent stack", p, c, item_count);
      }
   }
}

void benchmark_old_sequential_structures(int item_count) {
   benchmark_old_structure<queue<int>>(
       "Old queue", constants::SINGLE_THREAD, constants::SINGLE_THREAD, 
       item_count);
   benchmark_old_structure<stack<int>>(
       "Old stack", constants::SINGLE_THREAD, constants::SINGLE_THREAD, 
       item_count);
}

auto run_old_benchmarks(int item_count) -> void {
   benchmark_old_concurrent_queues(item_count);
   benchmark_old_concurrent_stacks(item_count);
   benchmark_old_sequential_structures(item_count);
}

template <typename StackType>
void create_stack_mutex_producers(StackType& stack, std::atomic<int>& produced_count, int items_per_producer, std::vector<std::jthread>& producers, int num_producers) {
   for (int p = 0; p < num_producers; ++p) {
      producers.emplace_back([&stack, &produced_count, items_per_producer]() {
         for (int i = 0; i < items_per_producer; ++i) {
            stack.mutex_push(i);
            produced_count.fetch_add(1);
         }
      });
   }
}

template <typename StackType>
void create_stack_cv_producers(StackType& stack, std::atomic<int>& produced_count, int items_per_producer, std::vector<std::jthread>& producers, int num_producers) {
   for (int p = 0; p < num_producers; ++p) {
      producers.emplace_back([&stack, &produced_count, items_per_producer]() {
         for (int i = 0; i < items_per_producer; ++i) {
            stack.cv_push(i);
            produced_count.fetch_add(1);
         }
      });
   }
}

template <typename StackType>
void create_stack_mutex_consumers(StackType& stack, std::atomic<int>& consumed_count, std::atomic<bool>& producers_done, int items_per_consumer, int total_items, std::vector<std::jthread>& consumers, int num_consumers) {
   for (int c = 0; c < num_consumers; ++c) {
      consumers.emplace_back([&stack, &consumed_count, &producers_done, items_per_consumer, total_items]() {process_stack_mutex_items(stack, consumed_count, producers_done, items_per_consumer, total_items);});
   }
}

template <typename StackType>
void process_stack_mutex_items(StackType& stack, std::atomic<int>& consumed_count, std::atomic<bool>& producers_done, int items_per_consumer, int total_items) {
   int items_processed = 0;
   while (items_processed < items_per_consumer) {
      auto item = stack.mutex_pop();
      if (item) {
         consumed_count.fetch_add(1);
         items_processed++;
      } else if (producers_done && consumed_count.load() >= total_items) {
         break;
      } else {
         std::this_thread::yield();
      }
   }
}

template <typename StackType>
void create_stack_cv_consumers(StackType& stack, std::atomic<int>& consumed_count, int items_per_consumer, std::vector<std::jthread>& consumers, int num_consumers) {
   for (int c = 0; c < num_consumers; ++c) {
      consumers.emplace_back([&stack, &consumed_count, items_per_consumer]() {
         for (int i = 0; i < items_per_consumer; ++i) {
            stack.cv_pop_wait();
            consumed_count.fetch_add(1);
         }
      });
   }
}

template <typename StackType>
void create_stack_producers(StackType& stack, std::atomic<int>& produced_count, int items_per_producer, std::vector<std::jthread>& producers, int num_producers, const std::string& method_name) {
   if (method_name == constants::MUTEX_METHOD) {
      create_stack_mutex_producers(stack, produced_count, items_per_producer, producers, num_producers);
   } else if (method_name == constants::CV_METHOD) {
      create_stack_cv_producers(stack, produced_count, items_per_producer, producers, num_producers);
   }
}

template <typename StackType>
void create_stack_consumers(StackType& stack, std::atomic<int>& consumed_count, std::atomic<bool>& producers_done, int items_per_consumer, int total_items, std::vector<std::jthread>& consumers, int num_consumers, const std::string& method_name) {
   if (method_name == constants::MUTEX_METHOD) {
      create_stack_mutex_consumers(stack, consumed_count, producers_done, items_per_consumer, total_items, consumers, num_consumers);
   } else if (method_name == constants::CV_METHOD) {
      create_stack_cv_consumers(stack, consumed_count, items_per_consumer, consumers, num_consumers);
   }
}

template <typename QueueType>
void create_queue_mutex_producers(QueueType& queue, std::atomic<int>& produced_count, int items_per_producer, std::vector<std::jthread>& producers, int num_producers) {
   for (int p = 0; p < num_producers; ++p) {
      producers.emplace_back([&queue, &produced_count, items_per_producer]() {
         for (int i = 0; i < items_per_producer; ++i) {
            queue.mutex_enqueue(i);
            produced_count.fetch_add(1);
         }
      });
   }
}

template <typename QueueType>
void create_queue_cv_producers(QueueType& queue, std::atomic<int>& produced_count, int items_per_producer, std::vector<std::jthread>& producers, int num_producers) {
   for (int p = 0; p < num_producers; ++p) {
      producers.emplace_back([&queue, &produced_count, items_per_producer]() {
         for (int i = 0; i < items_per_producer; ++i) {
            queue.cv_enqueue(i);
            produced_count.fetch_add(1);
         }
      });
   }
}

template <typename QueueType>
void process_queue_mutex_items(QueueType& queue, std::atomic<int>& consumed_count, std::atomic<bool>& producers_done, int items_per_consumer, int total_items) {
   int items_processed = 0;
   while (items_processed < items_per_consumer) {
      auto item = queue.mutex_dequeue();
      if (item) {
         consumed_count.fetch_add(1);
         items_processed++;
      } else if (producers_done && consumed_count.load() >= total_items) {
         break;
      } else {
         std::this_thread::yield();
      }
   }
}

template <typename QueueType>
void create_queue_mutex_consumers(QueueType& queue, std::atomic<int>& consumed_count, std::atomic<bool>& producers_done, int items_per_consumer, int total_items, std::vector<std::jthread>& consumers,
                                 int num_consumers) {
   for (int c = 0; c < num_consumers; ++c) {
      consumers.emplace_back([&queue, &consumed_count, &producers_done, items_per_consumer, total_items]() { process_queue_mutex_items(queue, consumed_count, producers_done, items_per_consumer, total_items);});
   }
}

template <typename QueueType>
void create_queue_cv_consumers(QueueType& queue, std::atomic<int>& consumed_count, int items_per_consumer, std::vector<std::jthread>& consumers, int num_consumers) {
   for (int c = 0; c < num_consumers; ++c) {
      consumers.emplace_back([&queue, &consumed_count, items_per_consumer]() {
         for (int i = 0; i < items_per_consumer; ++i) {
            queue.cv_dequeue_wait();
            consumed_count.fetch_add(1);
         }
      });
   }
}

template <typename QueueType>
void create_queue_producers(QueueType& queue, std::atomic<int>& produced_count, int items_per_producer, std::vector<std::jthread>& producers, int num_producers, const std::string& method_name) {
   if (method_name == constants::MUTEX_METHOD) {
      create_queue_mutex_producers(queue, produced_count, items_per_producer, producers, num_producers);
   } else if (method_name == constants::CV_METHOD) {
      create_queue_cv_producers(queue, produced_count, items_per_producer, producers, num_producers);
   }
}

template <typename QueueType>
void create_queue_consumers(QueueType& queue, std::atomic<int>& consumed_count, std::atomic<bool>& producers_done, int items_per_consumer, int total_items, std::vector<std::jthread>& consumers, int num_consumers, const std::string& method_name) {
   if (method_name == constants::MUTEX_METHOD) {
      create_queue_mutex_consumers(queue, consumed_count, producers_done, items_per_consumer, total_items, consumers, num_consumers);
   } else if (method_name == constants::CV_METHOD) {
      create_queue_cv_consumers(queue, consumed_count, items_per_consumer, consumers, num_consumers);
   }
}

template <typename LockFreeQueueType>
void create_lock_free_queue_producers(LockFreeQueueType& queue, std::atomic<int>& produced_count, int items_per_producer, std::vector<std::jthread>& producers, int num_producers) {
   producers.reserve(num_producers);
   for (int p = 0; p < num_producers; ++p) {
      producers.emplace_back([&queue, &produced_count, items_per_producer]() {
         for (int i = 0; i < items_per_producer; ++i) {
            queue.enqueue(i);
            produced_count.fetch_add(1);
         }
      });
   }
}

template <typename LockFreeQueueType>
void process_lock_free_queue_items(LockFreeQueueType& queue, std::atomic<int>& consumed_count, std::atomic<bool>& producers_done, int items_per_consumer, int total_items) {
   int items_processed = 0;
   while (items_processed < items_per_consumer) {
      int item;
      bool const succeeded = queue.try_dequeue(item);
      if (succeeded) {
         consumed_count.fetch_add(1);
         items_processed++;
      } else if (producers_done && consumed_count.load() >= total_items) {
         break;
      } else {
         std::this_thread::yield();
      }
   }
}

template <typename LockFreeQueueType>
void create_lock_free_queue_consumers(LockFreeQueueType& queue, std::atomic<int>& consumed_count, std::atomic<bool>& producers_done, int items_per_consumer, int total_items, std::vector<std::jthread>& consumers, int num_consumers) {
   consumers.reserve(num_consumers);
   for (int c = 0; c < num_consumers; ++c) {
      consumers.emplace_back([&queue, &consumed_count, &producers_done, items_per_consumer, total_items]() {
         process_lock_free_queue_items(queue, consumed_count, producers_done, items_per_consumer, total_items);
      });
   }
}

template <typename StackType>
void wait_for_completion(std::vector<std::jthread>& producers, std::vector<std::jthread>& consumers, std::atomic<bool>& producers_done) {
   for (auto& producer : producers) {
      producer.join();
   }
   producers_done = true;
   for (auto& consumer : consumers) {
      consumer.join();
   }
}

template <typename StackType>
auto measure_stack_benchmark_time(const std::string& stack_name, const std::string& method_name, int num_producers, int num_consumers, int items_per_producer) {
   StackType stack;
   std::atomic<int> produced_count = 0;
   std::atomic<int> consumed_count = 0;
   std::atomic<bool> producers_done = false;
   std::vector<std::jthread> producers;
   std::vector<std::jthread> consumers;
   int total_items = num_producers * items_per_producer;
   int const items_per_consumer = total_items / num_consumers;
   auto start_time = HighResClock::now();
   create_stack_producers(stack, produced_count, items_per_producer, producers, num_producers, method_name);
   create_stack_consumers(stack, consumed_count, producers_done, items_per_consumer, total_items, consumers, num_consumers, method_name);
   wait_for_completion<StackType>(producers, consumers, producers_done);
   auto end_time = HighResClock::now();
   return std::make_pair(std::chrono::duration_cast<MillisecDuration>(end_time - start_time), total_items);
}

template <typename StackType>
static auto benchmark_stack_new(const std::string& stack_name,const std::string& method_name, int num_producers, int num_consumers, int items_per_producer) -> void {
   auto [duration, total_items] = measure_stack_benchmark_time<StackType>(stack_name, method_name, num_producers, num_consumers, items_per_producer);
   std::print("{} with {} method: {} producers, {} consumers, {} items total - completed in {} ms\n", stack_name, method_name, num_producers, num_consumers, total_items, duration.count());
}

template <typename QueueType>
auto measure_queue_benchmark_time(const std::string& queue_name, const std::string& method_name, int num_producers, int num_consumers, int items_per_producer) {
   QueueType queue;
   std::atomic<int> produced_count = 0;
   std::atomic<int> consumed_count = 0;
   std::atomic<bool> producers_done = false;
   std::vector<std::jthread> producers;
   std::vector<std::jthread> consumers;
   int total_items = num_producers * items_per_producer;
   int const items_per_consumer = total_items / num_consumers;
   auto start_time = HighResClock::now();
   create_queue_producers(queue, produced_count, items_per_producer, producers, num_producers, method_name);
   create_queue_consumers(queue, consumed_count, producers_done, items_per_consumer, total_items, consumers, num_consumers, method_name);
   wait_for_completion<QueueType>(producers, consumers, producers_done);
   auto end_time = HighResClock::now();
   return std::make_pair(std::chrono::duration_cast<MillisecDuration>(end_time - start_time), total_items);
}

template <typename QueueType>
static auto benchmark_queue_new(const std::string& queue_name, const std::string& method_name, int num_producers, int num_consumers, int items_per_producer) -> void {
   auto [duration, total_items] = measure_queue_benchmark_time<QueueType>(queue_name, method_name, num_producers, num_consumers, items_per_producer);
   std::print("{} with {} method: {} producers, {} consumers, {} items total - completed in {} ms\n", queue_name, method_name, num_producers, num_consumers, total_items, duration.count());
}

template <typename LockFreeQueueType>
auto measure_lock_free_queue_time(const std::string& queue_name, const std::string& mechanism_name, int num_producers, int num_consumers, int items_per_producer) {
   LockFreeQueueType queue;
   std::atomic<int> produced_count = 0;
   std::atomic<int> consumed_count = 0;
   std::atomic<bool> producers_done = false;
   std::vector<std::jthread> producers;
   std::vector<std::jthread> consumers;
   int total_items = num_producers * items_per_producer;
   int const items_per_consumer = total_items / num_consumers;
   auto start_time = HighResClock::now();
   create_lock_free_queue_producers(queue, produced_count, items_per_producer, producers, num_producers);
   create_lock_free_queue_consumers(queue, consumed_count, producers_done, items_per_consumer, total_items, consumers, num_consumers);
   wait_for_completion<LockFreeQueueType>(producers, consumers, producers_done);
   auto end_time = HighResClock::now();
   return std::make_pair(std::chrono::duration_cast<MillisecDuration>(end_time - start_time), total_items);
}

template <typename LockFreeQueueType>
static auto benchmark_lock_free_queue_new(const std::string& queue_name, const std::string& mechanism_name, int num_producers, int num_consumers, int items_per_producer) -> void {
   auto [duration, total_items] = measure_lock_free_queue_time<LockFreeQueueType>(queue_name, mechanism_name, num_producers, num_consumers, items_per_producer);
   std::print("{} with {} mechanism: {} producers, {} consumers, {} items total - completed in {} ms\n", queue_name, mechanism_name, num_producers, num_consumers, total_items, duration.count());
}

void run_stack_benchmarks(int total_items) {
   std::print("Running Stack Benchmarks:\n========================\n\n");
   for (int const producers : constants::THREAD_COUNTS) {
      for (int const consumers : constants::THREAD_COUNTS) {
         int const items_per_producer = total_items / producers;
         benchmark_stack_new<vector_stack<int>>("vector_stack", constants::MUTEX_METHOD, producers, consumers, items_per_producer);
         benchmark_stack_new<vector_stack<int>>("vector_stack", constants::CV_METHOD, producers, consumers, items_per_producer);
         benchmark_stack_new<list_stack<int>>("list_stack", constants::MUTEX_METHOD, producers, consumers, items_per_producer);
         benchmark_stack_new<list_stack<int>>("list_stack", constants::CV_METHOD, producers, consumers, items_per_producer);
         std::print("\n");
      }
   }
}

void run_queue_benchmarks(int total_items) {
   std::print("\nRunning Queue Benchmarks:\n========================\n\n");
   for (int const producers : constants::THREAD_COUNTS) {
      for (int const consumers : constants::THREAD_COUNTS) {
         int const items_per_producer = total_items / producers;
         benchmark_queue_new<two_stack_queue<int>>("two_stack_queue", constants::MUTEX_METHOD, producers, consumers, items_per_producer);
         benchmark_queue_new<two_stack_queue<int>>("two_stack_queue", constants::CV_METHOD, producers, consumers, items_per_producer);
         std::print("\n");
      }
   }
}

void run_lock_free_queue_benchmarks(int total_items) {
   std::print("\nRunning Lock-Free Queue Benchmarks:\n========================\n\n");
   for (int const producers : constants::THREAD_COUNTS) {
      for (int const consumers : constants::THREAD_COUNTS) {
         int const items_per_producer = total_items / producers;
         benchmark_lock_free_queue_new<moodycamel::ConcurrentQueue<int>>("LockFreeQueue", constants::COMPARE_EXCHANGE, producers, consumers, items_per_producer);
         std::print("\n");
      }
   }
   benchmark_lock_free_queue_new<moodycamel::ReaderWriterQueue<int>>("LockFreeQueue", constants::ATOMIC_METHOD, constants::SINGLE_THREAD, constants::SINGLE_THREAD, total_items);
}

auto run_new_benchmarks(int N) -> void {
   run_stack_benchmarks(N);
   run_queue_benchmarks(N);
   run_lock_free_queue_benchmarks(N);
}

} // namespace benchmark