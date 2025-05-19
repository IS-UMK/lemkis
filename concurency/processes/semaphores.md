# Explanation of POSIX Semaphore

A **POSIX semaphore** is a synchronization primitive provided by the POSIX standard that allows threads or processes to synchronize their access to shared resources. It uses a counter to control access, ensuring mutual exclusion or controlled resource sharing.

---

## How POSIX Semaphore Works

### 1. **Key Concept**
- A semaphore is essentially a counter with operations to increment or decrement it atomically.
- It can be thought of as a "ticket counter" that allows threads or processes to acquire and release "tickets" to access a resource.

### 2. **Operations**
- **`sem_wait` (Decrement)**:
  - Decreases the semaphore counter.
  - If the counter is greater than 0, the operation proceeds immediately.
  - If the counter is 0, the calling process/thread blocks until another process/thread increments the semaphore using `sem_post`.
- **`sem_post` (Increment)**:
  - Increases the semaphore counter.
  - If there are processes/threads waiting on the semaphore, one of them is unblocked.
- **`sem_trywait`**:
  - Attempts to decrement the semaphore without blocking.
  - If the counter is 0, it returns immediately with an error.

### 3. **Types of Semaphores**
- **Named Semaphore**:
  - Identified by a name (e.g., `/my_semaphore`).
  - Used for inter-process synchronization.
  - Created using `sem_open` and destroyed using `sem_unlink`.
- **Unnamed Semaphore**:
  - Does not have a name and is usable only within a single process or between threads that share memory.
  - Created using `sem_init` and destroyed using `sem_destroy`.

### 4. **Blocking Behavior**
- If a thread/process calls `sem_wait` when the counter is 0, it blocks until another thread/process increments the semaphore using `sem_post`.

### 5. **Atomicity**
- All semaphore operations are atomic, ensuring that the counter updates correctly even with concurrent access.

---

## Lifecycle of a POSIX Semaphore

### 1. **Creation**
- **Named Semaphore**:
  ```c
  sem_t* sem = sem_open("/example_semaphore", O_CREAT | O_EXCL, 0644, 1);
  ```
  - `O_CREAT`: Create the semaphore if it doesn't already exist.
  - `O_EXCL`: Fail if the semaphore already exists.
  - `0644`: Permissions for the semaphore.
  - `1`: Initial value of the semaphore counter.

- **Unnamed Semaphore**:
  ```c
  sem_t sem;
  sem_init(&sem, 0, 1);
  ```
  - `&sem`: Address of the semaphore.
  - `0`: Semaphore is shared between threads (not between processes).
  - `1`: Initial value of the semaphore counter.

### 2. **Using the Semaphore**
- **Decrement (`sem_wait`)**:
  ```c
  sem_wait(semaphore);
  ```
  - Decrements the semaphore counter, blocking if the counter is 0.
- **Increment (`sem_post`)**:
  ```c
  sem_post(semaphore);
  ```
  - Increments the semaphore counter, unblocking a waiting process/thread if any.
- **Try Wait (`sem_trywait`)**:
  ```c
  if (sem_trywait(semaphore) == 0) {
      // Successfully decremented the semaphore
  } else {
      // Semaphore is already 0
  }
  ```

### 3. **Cleanup**
- **Named Semaphore**:
  ```c
  sem_close(semaphore);    // Close the semaphore descriptor
  sem_unlink("/example_semaphore"); // Remove it from the system
  ```
- **Unnamed Semaphore**:
  ```c
  sem_destroy(&sem); // Destroy the semaphore
  ```

---

## Use Cases

### 1. **Inter-Process Synchronization**
- Named semaphores allow multiple processes to synchronize access to shared resources, like shared memory or files.
- Example:
  - Parent process writes to shared memory.
  - Child process reads from shared memory after the parent signals.

### 2. **Thread Synchronization**
- Unnamed semaphores are used for synchronization between threads in the same process.

### 3. **Resource Management**
- Semaphores can be used to limit access to a finite resource, like a pool of database connections or a fixed number of hardware devices.

---

## Example: Named Semaphore for Inter-Process Synchronization

```c
#include <semaphore.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define SEM_NAME "/example_semaphore"

int main() {
    sem_t* sem = sem_open(SEM_NAME, O_CREAT, 0644, 0);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        printf("[Child] Waiting for semaphore...\n");
        sem_wait(sem); // Wait for the semaphore
        printf("[Child] Acquired semaphore!\n");
        sem_close(sem);
        exit(EXIT_SUCCESS);
    } else {
        // Parent process
        printf("[Parent] Doing some work...\n");
        sleep(2);
        printf("[Parent] Signaling semaphore...\n");
        sem_post(sem); // Signal the semaphore
        wait(NULL);    // Wait for child process
        sem_close(sem);
        sem_unlink(SEM_NAME);
    }

    return 0;
}
```

---

## Output of Example

```
[Parent] Doing some work...
[Child] Waiting for semaphore...
[Parent] Signaling semaphore...
[Child] Acquired semaphore!
```

---

# C++ class implementation



```cpp
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

```
