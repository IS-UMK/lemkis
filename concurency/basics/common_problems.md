

# Common Problems 

## What is a Race Condition?

A race condition occurs when two or more threads access shared data simultaneously, and at least one of the threads modifies the data. Without proper synchronization, this can lead to unpredictable and incorrect behavior. Race conditions are common in multithreaded programs and can be challenging to debug.

---


## Code Example: Race Condition

The following example demonstrates a race condition where multiple threads increment a shared counter without proper synchronization:


```cpp
#include <iostream>
#include <thread>
#include <vector>

int counter = 0; // Shared resource

void incrementCounter() {
    for (int i = 0; i < 1000; ++i) {
        ++counter; // Critical section
    }
}

int main() {
    const int numThreads = 10;
    std::vectorstd::thread threads;

    text
    // Create multiple threads
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(incrementCounter);
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Final counter value: " << counter << std::endl;
    return 0;
}
```
### Expected Output:
If there were no race condition, the final value of `counter` would be `10000` (`10 threads * 1000 increments per thread`).

### Possible Output:
Due to the race condition, the actual output might be less than `10000`, as some increments are lost when multiple threads overwrite each other's updates to `counter`.

---

## Why Does This Happen?

The statement `++counter` is not atomic. It involves three steps:
1. Reading the current value of `counter`.
2. Incrementing the value.
3. Writing the new value back to memory.

If two threads execute this operation simultaneously, they may both read the same initial value of `counter`, increment it, and write back the same result, effectively losing one increment.

---

## Fixing the Race Condition

To fix this issue, you can use a **mutex** to protect the critical section:
```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>

int counter = 0; // Shared resource
std::mutex mtx; // Mutex for synchronization

void incrementCounter() {
    for (int i = 0; i < 1000; ++i) {
        std::lock_guard lock(mtx); // Lock the mutex
        ++counter;
    }
}

int main() {
    const int numThreads = 10;
    std::vectorstd::thread threads;

    // Create multiple threads
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(incrementCounter);
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "Final counter value: " << counter << std::endl;
    return 0;
}
```
# Mutexes 

`std::mutex` is a synchronization primitive in C++ that allows only one thread to execute a critical section of code at a time. It is used to protect shared data from being accessed simultaneously by multiple threads, ensuring that the data remains consistent and preventing race conditions.


## Key Features of `std::mutex`

1. **Mutual Exclusion**: Only one thread can lock the mutex at a time, preventing other threads from accessing the protected data until the lock is released.
2. **Blocking Behavior**: If a thread attempts to lock a mutex that is already locked by another thread, it will block until the mutex is unlocked.
3. **Methods**:
   - **`lock()`**: Blocks the calling thread until it acquires the mutex.
   - **`try_lock()`**: Attempts to acquire the mutex without blocking; returns `true` if successful, `false` otherwise.
   - **`unlock()`**: Releases the mutex, allowing other threads to acquire it.


## Problems?
Mutexes are essential for synchronization in multithreaded programming, but they can also lead to several common problems if not used correctly. Here are some examples of these issues and how they can be addressed.


## Deadlock

A deadlock occurs when two or more threads are blocked indefinitely, each waiting for the other to release a resource.

### Example of Deadlock

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex m1, m2;

void thread1() {
    std::lock_guard lock1(m1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
    std::lock_guard lock2(m2); // Deadlock risk
    std::cout << "Thread 1 finished\n";
}

void thread2() {
    std::lock_guard lock1(m2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
    std::lock_guard lock2(m1); // Deadlock risk
    std::cout << "Thread 2 finished\n";
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);

    t1.join();
    t2.join();

return 0;
}


```
## Using `std::scoped_lock` to Prevent Deadlocks
`std::scoped_lock` is a powerful tool in C++ that helps prevent deadlocks by ensuring that multiple mutexes are locked in a consistent and deadlock-safe manner. It is specifically designed to address scenarios where multiple mutexes need to be locked simultaneously.
Note that `std::scoped_lock` is a **c++17** feature making it a modern and recommended approach for managing multiple locks.

## How `std::scoped_lock` Works

- **Atomic Locking**: Internally uses `std::lock`, which atomically locks multiple mutexes. This ensures that no circular wait conditions occur.
- **Automatic Resource Management**: When the `std::scoped_lock` object goes out of scope, it automatically unlocks all the mutexes it locked, ensuring proper resource cleanup.
- **Consistent Order**: By locking all mutexes at once, `std::scoped_lock` avoids inconsistent locking orders across threads, which is a common cause of deadlocks.

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex m1, m2;

void thread1() {
    // Use std::scoped_lock to lock both mutexes in a deadlock-safe manner
    std::scoped_lock lock(m1, m2);
    std::cout << "Thread 1 acquired both mutexes\n";
}

void thread2() {
    // Use std::scoped_lock to lock both mutexes in a deadlock-safe manner
    std::scoped_lock lock(m1, m2);
    std::cout << "Thread 2 acquired both mutexes\n";
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);

    t1.join();
    t2.join();

    return 0;
}

```

## Starvation

Starvation occurs when a thread is unable to gain access to a shared resource due to other threads holding onto it for extended periods.

### Example of Starvation

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void highPriorityTask() {
    while (true) {
        std::lock_guard lock(mtx);
        std::cout << "High-priority task running\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void lowPriorityTask() {
    while (true) {
        if (mtx.try_lock()) { // Non-blocking attempt to acquire the lock
            std::cout << "Low-priority task running\n";
            mtx.unlock();
            break;
        } else {
            // Retry after some delay
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

int main() {
    std::thread t1(highPriorityTask);
    std::thread t2(lowPriorityTask);


    t1.detach();
    t2.join();

    return 0;
}
```
### Why Starvation Occurs

- **Continuous Locking by High-Priority Task**: The high-priority task continuously locks the mutex, holds it for a short duration, and then releases it. This creates a scenario where the mutex is frequently available but only for brief periods.
- **Low-Priority Task Struggles to Acquire Mutex**: The low-priority task tries to lock the mutex but often finds it already locked by the high-priority task. It retries after a short delay, but because the high-priority task is so active, the low-priority task may struggle to acquire the mutex.
- **Starvation Condition**: If the high-priority task's locking frequency is high enough, it can prevent the low-priority task from ever acquiring the mutex, leading to starvation.
## Livelock

A livelock occurs when two or more threads are unable to proceed because they keep trying to access a shared resource but fail due to continuous interference from other threads. Unlike a deadlock, where threads are blocked indefinitely, in a livelock, threads are actively trying to access the resource but are unable to make progress.

### Example of Livelock

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <atomic>

std::mutex mtx;
std::atomic<bool> resourceAvailable{false};

void threadFunction(int id) {
    while (!resourceAvailable.load()) {
        if (mtx.try_lock()) {
            if (!resourceAvailable.load()) {
                std::cout << "Thread " << id << " releasing lock\n";
                mtx.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Retry later
            } else {
                mtx.unlock();
                break;
            }
        }
        // Busy-waiting without progress
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
    }
}

int main() {
    std::thread t1(threadFunction, 1);
    std::thread t2(threadFunction, 2);

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    resourceAvailable.store(true); // Make resource available

    t1.join();
    t2.join();

    return 0;
}
```