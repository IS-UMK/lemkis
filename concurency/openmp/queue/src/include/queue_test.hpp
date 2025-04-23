#pragma once

// Deklaracje funkcji testujących
void run_sequential_ompqueue_test(int producers, int consumers);
void run_parallel_ompqueue_test(int producers, int consumers);
void run_jthread_concurrentqueue_test(int producers, int consumers);
