# Basic notions

## Virtual memory

**Virtual memory** is a memory management technique used by modern operating systems to give applications the illusion that they have access to a large, 
continuous block of memory, even if the physical RAM is limited or fragmented. It achieves this 
by using both the computer's physical memory (RAM) and a portion of secondary storage (such as a hard drive or SSD) as an extension of RAM


## Memory page

**page** (also called a **memory page** or **virtual page**) is a fixed-length, contiguous block of virtual memory that serves as the smallest unit of data for memory management in operating systems that use virtual memory. Each page is described by a single entry in a page table, which maps virtual pages to physical memory locations (called page frames).

The operating system divides memory into these pages to efficiently manage, allocate, and move data. When a program needs more memory than is physically available, the OS can transfer pages between main memory (RAM) and secondary storage (such as a hard disk) in a process called paging or swapping.

Pages are typically 4 KB or 8 KB in size, but the exact size depends on the system architecture. The use of pages allows the OS to provide each process with its own continuous virtual address space, even if the underlying physical memory is fragmented or limited.

## Shared memory

**Shared memory** is a fundamental inter-process communication (IPC) mechanism that enables multiple processes to access the same region of memory, allowing them to exchange data efficiently without redundant copies or complex message-passing protocols. Here's a structured breakdown:

### Core Characteristics

1. Simultaneous Access: Multiple processes can read/write the same memory region concurrently.
2. Efficiency: Avoids data copying between processes (unlike pipes/sockets).
3. Persistence: Survives process termination until explicitly deleted (shm_unlink).
4. Virtual Filesystem: On Linux, POSIX shared memory objects reside in /dev/shm (a tmpfs RAM disk).


### Key Advantages
1. Performance: Direct memory access eliminates serialization/deserialization overhead.
2. Atomic Operations: Processes can use synchronization primitives (semaphores) with shared memory.
3. Large Data Handling: Suitable for sharing substantial datasets between process

### Use Cases
1. High-Performance Computing:
2. Real-time data processing pipelines.
3. Multi-processor task coordination.
4. Database Systems: Shared query caches.
5. Gaming/Graphics: Texture/asset sharing between processes.

### Critical Considerations
1. Synchronization: Requires mutexes/semaphores to prevent race conditions.
2. Security: Permissions (0600 in shm_open) control access.
3. Resource Management: Always munmap and shm_unlink to prevent memory leaks. Use ftruncate cautiously to avoid oversized allocations

## POSIX Shared Memory Functions  

The core functions for POSIX shared memory management form a workflow for creating, accessing, and cleaning up shared memory regions. Here's the essential toolkit:

### 1. `int shm_open(const char *name, int oflag, mode_t mode);`  
**Purpose**: Creates/opens a shared memory object (persistent or temporary).  
**Params** 
```cpp
const char *name: "/my_shm" // Must begin with '/'
int oflag: O_CREAT | O_RDWR // Common flags
mode_t mode: 0600 // User read/write permissions
```
**Remainder**
`shm_open` returns a file descriptor because POSIX shared memory objects are designed to integrate seamlessly with the existing UNIX file and memory management infrastructure. In UNIX-like systems, a file descriptor is a universal handle that represents an open file, device, pipe, or other resources—including shared memory objects. This design choice offers several advantages:

1. **Uniformity**: By representing shared memory as a file descriptor, the same system calls and APIs (like mmap, ftruncate, close) used for regular files can also be used for shared memory. This simplifies the API and makes it easier for developers to manage different types of resources in a consistent way.
2. **Resource Management**: File descriptors provide a well-understood mechanism for tracking resource usage, permissions, and cleanup. When a shared memory object is opened, the kernel creates an "open file description" and assigns a file descriptor to it, just like with files.
3. **Interoperability**: Many system calls that operate on files (such as mmap for mapping memory) require a file descriptor as an argument. By returning a file descriptor, shm_open allows shared memory objects to be mapped into a process's address space using the same mechanisms as files.
4. **Precedent**: In POSIX and UNIX, file descriptors are already used for various object types, including regular files, pipes, sockets, and devices. Extending this model to shared memory objects keeps the system interface simple and extensible.

**Behavior**:  
- Returns file descriptor for memory object  
- Analogous to `open()` for files  
- Creates persistent object if `O_CREAT` specified  

### 2. `shm_unlink("/my_shm"); // Immediate name removal`
**Purpose**: Removes shared memory object name  


## Complementary System Calls
### 3. `int ftruncate(int fd, off_t length);`
**Purpose**: Sets shared memory size  
**Usage**:  
```cpp
ftruncate(shm_fd, 4096); // Set 4KB size
// Required after creation to allocate space
```
### 4. `void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);`
**Purpose**: Maps memory into address space  
**Typical Usage**:
```cpp
char *ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
// MAP_SHARED ensures cross-process visibility
```
### 5. `int munmap(void *addr, size_t length);`
**Purpose**: Unmaps memory region  
**Usage**:  
```cpp
munmap(ptr, 4096); // Release virtual mapping
// Doesn't affect other process mappings
```


### 6. `int close(int fd);`
**Purpose**: Releases file descriptor  
**Signature**:  

## Complete Workflow Example


**Typical Initialization Sequence**:  
```cpp
// 1. Create shared memory
int fd = shm_open("/shm", O_CREAT|O_RDWR, 0600);

// 2. Allocate size
ftruncate(fd, 4096);

// 3. Map memory
void *ptr = mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

// 4. Cleanup name
shm_unlink("/shm");

// 5. Release descriptor
close(fd);

// ... Use memory ...

// 6. Unmap when done
munmap(ptr, 4096);
```

**Cleanup Order**:  
1. `munmap()` all memory regions  
2. `close()` associated file descriptors  
3. `shm_unlink()` names (preferably early)  

**Key Features**:  
- **Persistence**: Objects survive process termination unless unlinked  
- **Security**: Permissions controlled via `mode` parameter  
- **Efficiency**: Shared memory provides fastest IPC mechanism  
- **Atomicity**: OS guarantees full page transfers  

> **Pro Tip**: Always check return values for errors in production code. Use `perror()` or `strerror(errno)` for debugging.  
> **Warning**: Never store pointers in shared memory - virtual addresses differ between processes. Use offsets instead.

### Error handling

```cpp
#pragma once


#include <format>
#include <iostream>
#include <system_error>
#include <cstdlib>
#include <cerrno>

template <typename... Args>
[[noreturn]] void syserr(std::format_string<Args...> fmt, Args&&... args) {
    std::error_code ec(errno, std::system_category());
    auto user_msg{std::format(fmt, std::forward<Args>(args)...)};
    std::cerr << std::format(
        "ERROR: {}\n ({}; {})\n",
        user_msg, // user message formatted inline
        ec.value(),
        ec.message()
    );
    throw std::system_error(ec, user_msg);
    
}


// Example usage: if (open("file.txt", O_RDONLY) == -1) syserr("File open failed");
```


## Example

```cpp
#include <array>
#include <chrono>
#include <cstring>
#include <exception>
#include <fcntl.h> // For O_* constants
#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h> // For mode constants
#include <thread>
#include <unistd.h>
#include <vector>
#include <wait.h>



void print_table(std::string_view t) {
  std::println("Process {}, table at address {}:\n{}\n", getpid(),
               static_cast<const void *>(t.data()),
               t | std::views::chunk(1) | std::views::join_with('|') |
                   std::ranges::to<std::string>());
}

class shared_memory {
public:
  shared_memory(std::string_view name, size_t size) : name_(name), size_(size) {
    fd_memory_ = shm_open(name_.data(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_memory_ == -1) {
      throw std::runtime_error("shm_open failed");
    }

    if (ftruncate(fd_memory_, size_) == -1) {
      shm_unlink(name_.data());
      throw std::runtime_error("ftruncate failed");
    }

    mapped_mem_ = static_cast<char *>(mmap(
        nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_memory_, 0));
    if (mapped_mem_ == MAP_FAILED) {
      shm_unlink(name_.data());
      throw std::runtime_error("mmap failed");
    }

    close(fd_memory_);
    shm_unlink(
        name_.data()); // Remove the special file; memory is still accessible
  }

  ~shared_memory() {
    if (mapped_mem_ != MAP_FAILED) {
      munmap(mapped_mem_, size_);
    }
  }

  char *get() const { return mapped_mem_; }
  size_t size() const { return size_; }

private:
  std::string_view name_;
  size_t size_;
  int fd_memory_;
  char *mapped_mem_;
};


constexpr std::string_view SHM_NAME = "/practice_memory";
constexpr int NAP_TIME = 2;
constexpr std::string_view BUFF{"Ala ma kota"};
constexpr size_t BUFF_SIZE = BUFF.size();


int main() {
  try {
    
    std::print("Page size is {} bytes\n", sysconf(_SC_PAGE_SIZE));

    shared_memory shared_mem(SHM_NAME, BUFF_SIZE);
    char *mapped_mem = shared_mem.get();

    print_table(std::string_view(mapped_mem, BUFF_SIZE));

    std::this_thread::sleep_for(std::chrono::seconds(NAP_TIME));

    const pid_t pid = fork();
    const std::string_view mapped_mem_view(mapped_mem, BUFF_SIZE);
    if (pid == -1) {
      throw std::runtime_error("fork failed");
    } else if (pid == 0) {
      // Child process
      std::this_thread::sleep_for(std::chrono::seconds(NAP_TIME));
      print_table(mapped_mem_view);
      std::this_thread::sleep_for(std::chrono::seconds(2 * NAP_TIME));
      print_table(mapped_mem_view);
      return 0;
    } else {
      // Parent process
      std::print("Parent PID: {}, Child PID: {}\n", getpid(), pid);
      print_table(mapped_mem_view);
      std::this_thread::sleep_for(std::chrono::seconds(2 * NAP_TIME));

      std::ranges::copy(BUFF, mapped_mem);
      std::print("Process {}, modified shared memory content\n", getpid());
      print_table(mapped_mem_view);

      wait(nullptr); // Wait for child process
    }
  } catch (const std::exception &ex) {
    std::print("Error: {}\n", ex.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
```
Note that implicitly **shared memory inheritance** occurs that is the parent process uses mmap() to map the shared memory before the fork() call.
When fork() is called, the child process inherits the parent's memory mappings (including the shared memory region) as part of the process duplication.
Both the parent and child directly share the same memory mapping without needing to re-open or re-map the shared memory.

### Why `wait(nullptr)` is Used in the Code

The `wait(nullptr)` function is used to wait for a child process to terminate. It serves an important purpose in process management when dealing with forked child processes in a program.

Is this program correct? Does this code works? If not can you guess why not and can you fix it (it might be profitable to go to [semaphores](https://github.com/IS-UMK/lemkis/blob/master/concurency/processes/semaphores.md)). 

---

### 1. **Reaping the Child Process**
When a parent process creates a child process using `fork()`, the operating system keeps information about the child process (such as its exit status) in a data structure in memory until the parent explicitly reads it. 

- **Zombie Process**: A child process that has terminated but still occupies an entry in the process table because its parent has not yet retrieved its exit status.
- By calling `wait(nullptr)`, the parent process reads the child's exit status and allows the operating system to clean up the data structure for the child process, preventing it from becoming a zombie.

---

### 2. **Blocking Until the Child Completes**
`wait(nullptr)` causes the parent process to block (pause execution) until one of its child processes terminates. This ensures that the parent doesn't proceed with operations that might depend on the child process having completed its work.

In the given code:
```cpp
wait(nullptr); // Wait for child process
```
- The parent process waits for the child process to complete before continuing, ensuring that the shared memory modifications by the child process are finished before the parent exits or cleans up resources.

---

### 3. **Using `nullptr` as the Argument**
The `wait()` function accepts a pointer to an integer argument where it can store the exit status of the terminated child process. By passing `nullptr`, you indicate that you don't care about the exit status and simply want to wait for the child to terminate.

If you want to retrieve the exit status, you can pass a pointer to an integer instead:
```cpp
int status;
wait(&status); // Wait for the child process and store its exit status in 'status'
```

---

### 4. **Why It's Important**
If you don't call `wait()` or a similar function (e.g., `waitpid()`), the following issues can occur:
1. **Zombie Processes**: The child process will remain a zombie in the process table, leading to resource leaks.
2. **Undefined Behavior**: The parent process might proceed without knowing whether the child process has completed, potentially leading to race conditions or resource cleanup issues.

---

### 5. **Alternative: `waitpid()`**
`waitpid()` offers more control over which child process to wait for:
```cpp
#include <sys/wait.h>
waitpid(pid, nullptr, 0); // Wait for a specific child process with PID 'pid' to terminate
```
- This is useful if the parent process has multiple child processes and wants to wait for a specific one.

---


# SOlution to homework - synchronization on shared memory

```cpp
#include <array>
#include <chrono>
#include <cstring>
#include <exception>
#include <fcntl.h> // For O_* constants
#include <format>
#include <iostream>
#include <print>
#include <ranges>
#include <semaphore>
#include <string>
#include <string_view>
#include <sys/mman.h>
#include <sys/stat.h> // For mode constants
#include <thread>
#include <unistd.h>
#include <vector>
#include <wait.h>

#include <semaphore.h>
#include <stdexcept>
#include <string>

void print_table(std::string_view t) {
  std::println("Process {}, table at address {}:\n{}\n", getpid(),
               static_cast<const void *>(t.data()),
               t | std::views::chunk(1) | std::views::join_with('|') |
                   std::ranges::to<std::string>());
}

class shared_memory {
public:
  shared_memory(std::string_view name, size_t size) : name_(name), size_(size) {
    fd_memory_ = shm_open(name_.data(), O_CREAT | O_RDWR, S_IRUSR | S_IWUSR);
    if (fd_memory_ == -1) {
      throw std::runtime_error("shm_open failed");
    }

    if (ftruncate(fd_memory_, size_) == -1) {
      shm_unlink(name_.data());
      throw std::runtime_error("ftruncate failed");
    }

    mapped_mem_ = static_cast<char *>(mmap(
        nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_memory_, 0));
    if (mapped_mem_ == MAP_FAILED) {
      shm_unlink(name_.data());
      throw std::runtime_error("mmap failed");
    }

    close(fd_memory_);
    shm_unlink(
        name_.data()); // Remove the special file; memory is still accessible
  }

  ~shared_memory() {
    if (mapped_mem_ != MAP_FAILED) {
      munmap(mapped_mem_, size_);
    }
  }

  char *get() const { return mapped_mem_; }
  size_t size() const { return size_; }

private:
  std::string_view name_;
  size_t size_;
  int fd_memory_;
  char *mapped_mem_;
};

class named_posix_semaphore {
public:
  explicit named_posix_semaphore(std::string_view name,
                                 unsigned int initial_value = 0)
      : name_(name), semaphore_(nullptr) {
    // Create or open the semaphore
    semaphore_ = sem_open(name.data(), O_CREAT | O_EXCL, 0644, initial_value);
    if (semaphore_ == SEM_FAILED) {
      throw std::runtime_error(
          std::format("Failed to create semaphore: {}", name));
    }
  }

  ~named_posix_semaphore() {
    // Close the semaphore
    if (semaphore_ != nullptr) {
      sem_close(semaphore_);
    }

    // Unlink the semaphore from the system
    sem_unlink(name_.data());
  }

  // Copy constructor and copy assignment are deleted to avoid accidental
  // copying
  named_posix_semaphore(const named_posix_semaphore &) = delete;
  named_posix_semaphore &operator=(const named_posix_semaphore &) = delete;

  // Move constructor
  named_posix_semaphore(named_posix_semaphore &&other) noexcept
      : name_(std::move(other.name_)), semaphore_(other.semaphore_) {
    other.semaphore_ = nullptr;
  }

  // Move assignment operator
  named_posix_semaphore &operator=(named_posix_semaphore &&other) noexcept {
    if (this != &other) {
      // Clean up existing semaphore
      if (semaphore_ != nullptr) {
        sem_close(semaphore_);
        sem_unlink(name_.data());
      }

      // Move resources
      name_ = std::move(other.name_);
      semaphore_ = other.semaphore_;
      other.semaphore_ = nullptr;
    }
    return *this;
  }

  // Wait (decrement semaphore)
  void wait() {
    if (sem_wait(semaphore_) == -1) {
      throw std::runtime_error(
          std::format("Failed to wait on semaphore: ", name_));
    }
  }

  // Signal (increment semaphore)
  void post() {
    if (sem_post(semaphore_) == -1) {
      throw std::runtime_error(
          std::format("Failed to post to semaphore: ", name_));
    }
  }

  // Try to wait without blocking
  bool try_wait() { return sem_trywait(semaphore_) == 0; }

private:
  std::string_view name_; // Name of the semaphore
  sem_t *semaphore_;      // POSIX semaphore handle
};

constexpr std::string_view SHM_NAME = "/practice_memory";
constexpr std::string_view SEM_NAME = "/semaphore_name";
constexpr int NAP_TIME = 2;
constexpr std::string_view BUFF{"Ala ma kota"};
constexpr size_t BUFF_SIZE = BUFF.size();

inline std::counting_semaphore<1>
    sync_semaphore(0); // Initialize the semaphore with 0.
int main() {
  try {

    std::print("Page size is {} bytes\n", sysconf(_SC_PAGE_SIZE));

    shared_memory shared_mem(SHM_NAME, BUFF_SIZE);
    char *mapped_mem = shared_mem.get();

    print_table(std::string_view(mapped_mem, BUFF_SIZE));

    std::this_thread::sleep_for(std::chrono::seconds(NAP_TIME));
    named_posix_semaphore sem{SEM_NAME};
    const pid_t pid = fork();
    const std::string_view mapped_mem_view(mapped_mem, BUFF_SIZE);
    if (pid == -1) {
      throw std::runtime_error("fork failed");
    } else if (pid == 0) {
      // Child process
      // std::this_thread::sleep_for(std::chrono::seconds(NAP_TIME));
      // print_table(mapped_mem_view);
      // std::this_thread::sleep_for(std::chrono::seconds(2 * NAP_TIME));
      // sync_semaphore.acquire();
      sem.wait();
      std::println("Child, acquired");
      print_table(mapped_mem_view);
      return 0;
    } else {
      // Parent process
      std::println("Parent PID: {}, Child PID: {}", getpid(), pid);
      // print_table(mapped_mem_view);
      std::this_thread::sleep_for(std::chrono::seconds(2 * NAP_TIME));

      std::ranges::copy(BUFF, mapped_mem);
      std::println("Process {}, modified shared memory content", getpid());
      print_table(mapped_mem_view);
      // sync_semaphore.release();
      sem.post();
      wait(nullptr); // Wait for child process
    }
  } catch (const std::exception &ex) {
    std::print("Error: {}\n", ex.what());
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
```

