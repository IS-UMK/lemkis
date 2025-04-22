#include "include/own_test.hpp"
#include "include/queue_test.hpp"
#include <vector>

int main() {
    const std::vector<int> producer_counts = {1, 2, 4};
    const std::vector<int> consumer_counts = {1, 2, 4};

     for (int producers : producer_counts) {
         for (int consumers : consumer_counts) {
             run_sequential_ompqueue_test(producers, consumers);
             run_parallel_ompqueue_test(producers, consumers);
             //run_jthread_concurrentqueue_test(producers, consumers);
         }
     }
    run_own_test();

    return 0;
}
