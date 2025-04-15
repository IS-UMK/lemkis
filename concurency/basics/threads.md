# Introduction to `std::thread` and `std::jthread` in C++

## Overview

Modern C++ provides robust support for multithreading through the `<thread>` library, introduced in C++11. Two key classes for creating and managing threads are:

- **`std::thread`**: A lightweight class for basic thread management.
- **`std::jthread`**: Introduced in C++20, it improves upon `std::thread` by adding features like automatic joining and cooperative stopping.

This introduction will cover the basics of both `std::thread` and `std::jthread`, along with examples to illustrate their usage.

---

## `std::thread`

### Description
`std::thread` represents a single thread of execution. Threads begin execution immediately upon construction, running the function or callable object provided as an argument. The user must explicitly manage the thread's lifecycle by calling `.join()` or `.detach()`.

### Key Features
- **Explicit Lifecycle Management**: Users must ensure that threads are joined or detached before destruction.
- **Lightweight**: Minimal overhead compared to `std::jthread`.

### Example: Using `std::thread`

```cpp
#include <iostream>
#include <thread>

void printMessage(const std::string& message) {
    std::cout << "Thread says: " << message << std::endl;
}

int main() {
    std::string msg = "Hello from std::thread!";
    std::thread t(printMessage, msg); // Start a new thread

    t.join(); // Wait for the thread to finish
    return 0;
}
```


### Important Notes
1. If `.join()` or `.detach()` is not called before the thread object is destroyed, the program will terminate with an exception.
2. Use `.detach()` if you want the thread to run independently, but ensure no resources used by the thread are destroyed prematurely.

---

## `std::jthread`

### Description
`std::jthread` (short for "joining thread") improves upon `std::thread` by:
1. Automatically joining the thread when the `jthread` object goes out of scope.
2. Adding a built-in cooperative stopping mechanism using `std::stop_token`.

### Key Features
- **Automatic Joining**: No need to explicitly call `.join()`.
- **Cooperative Stopping**: Allows threads to check for stop requests using `std::stop_token`.

### Example: Using `std::jthread`

```cpp
#include <iostream>
#include <thread>
#include <stop_token>

void printRepeatedly(std::stop_token st) {
    while (!st.stop_requested()) {
        std::cout << "Thread is running...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    std::cout << "Thread stopped.\n";
}

int main() {
    std::jthread t(printRepeatedly); // Start a new thread
    std::this_thread::sleep_for(std::chrono::seconds(2));
    t.request_stop(); // Request the thread to stop

    return 0; // Automatically joins the thread
}
```


### Advantages of `std::jthread`
1. Simplifies resource management by automatically joining threads.
2. Provides a clean and efficient way to implement cooperative stopping.

---

## Comparison of `std::thread` and `std::jthread`

| Feature                     | `std::thread`                         | `std::jthread`                        |
|-----------------------------|----------------------------------------|---------------------------------------|
| **Lifecycle Management**    | Manual (`join()` or `detach()`).       | Automatic (`join()` on destruction).  |
| **Stopping Mechanism**      | No built-in mechanism.                | Supports cooperative stopping via `std::stop_token`. |
| **Ease of Use**             | Requires careful management.          | Simplifies thread management.         |
| **Performance Overhead**    | Lightweight.                          | Slightly heavier due to stop tokens.  |
| **Availability**            | Introduced in C++11.                  | Introduced in C++20.                  |

---

## When to Use Each

- Use **`std::thread`**:
  - When you need lightweight threads without additional features.
  - In performance-critical applications where every bit of overhead matters.
  - For legacy codebases where C++20 features are unavailable.
  - In scenarios where detached threads are beneficial, `std::thread` must be used instead of `std::jthread`, as `std::jthread` does not support detaching

- Use **`std::jthread`**:
  - For modern applications where ease of use and safety are priorities.
  - When you need cooperative stopping mechanisms.
  - To avoid manual lifecycle management (e.g., forgetting to call `.join()`).

---
## Real-Life Example of Using `std::thread::detach`

### Introduction

In C++, `std::thread::detach` is used to separate a thread from its thread handle, allowing it to run independently until its completion. Here's a practical example where detaching a thread is useful:

### Example: Background Logging Service

Imagine you're developing a web server that needs to log events in the background without blocking the main server thread. You can use a detached thread to handle logging independently.

### Code Example
Imagine you're developing a web server that needs to log events in the background without blocking the main server thread. You can use a detached thread to handle logging independently.
```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>

void logEvents(const std::string& logFile) {
    std::ofstream logStream(logFile);
    if (logStream.is_open()) {
        while (true) {
            // Simulate logging events
            logStream << "Logging event...\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    } else {
        std::cerr << "Failed to open log file.\n";
    }
}

int main() {
    std::string logFilePath = "server.log";
    std::thread loggingThread(logEvents, logFilePath);
    loggingThread.detach(); // Detach the logging thread

    // Main server loop
    while (true) {
        std::cout << "Server is running...\n";
        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}

```


### Why Detach is Used Here

1. **Independence**: The logging thread runs independently of the main server thread, ensuring that logging does not interfere with server operations.
2. **No Need for Synchronization**: Since the logging thread is detached, there's no need to worry about joining it or ensuring it finishes before the server exits.
3. **Resource Management**: The detached thread manages its own resources (e.g., the log file stream), and any allocated resources are freed when the thread completes.

However, keep in mind that using `detach` means you won't be able to wait for the thread to finish or handle its completion explicitly. If your program exits before the detached thread completes, it may result in abnormal termination.

---

### Real-World Scenarios

- **Background Tasks**: Detached threads are useful for tasks like periodic cleanup, monitoring, or maintenance that should run independently of the main application flow.
- **Asynchronous Operations**: In scenarios where asynchronous operations need to be performed without blocking the main thread, detaching threads can be beneficial.
- **Long-Running Processes**: For processes that need to run indefinitely or for an extended period, detaching allows them to continue running even if the main program exits.

### Caution

- **Abnormal Termination**: If a detached thread is still running when the main program exits, it may lead to abnormal process termination.
- **Resource Leaks**: Ensure that detached threads properly manage resources to avoid leaks.

In summary, `detach` is useful when you need threads to run independently without affecting the main program flow, but it requires careful resource management to avoid issues.

## Summary

- **`std::thread`** is a basic threading class that requires explicit management of its lifecycle.
- **`std::jthread`** simplifies multithreading by handling automatic joining and providing built-in support for cooperative stopping.
- Both classes allow developers to write concurrent programs efficiently, but choosing between them depends on your application's requirements and the availability of C++20 features.
