#pragma once

// Deklaracje funkcji testuj¹cych
void run_sequential_ompqueue_test(int producers, int consumers);
void run_parallel_ompqueue_test(int producers, int consumers);
void run_jthread_concurrentqueue_test(int producers, int consumers);
