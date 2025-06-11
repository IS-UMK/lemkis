# Creation of new processes and usage of `fork()`

---

## **What is a Process?**

A **process** is an instance of a running program. It consists of:
1. **Code**: The program's instructions.
2. **Data**: Variables, memory, etc.
3. **Execution Context**: Registers, program counter, stack, etc.

Operating systems allow us to create new processes, enabling multitasking and parallel execution of programs.

---

## **How to Create New Processes**

In most operating systems, processes are created using the **`fork()`** system call (or equivalent). In POSIX-compliant systems, `fork()` is the primary way to create new processes.

### **`fork()` Overview**
- **Definition**: `fork()` is a system call that creates a new process by duplicating the calling process.
- **Parent Process**: The process that calls `fork()`.
- **Child Process**: The new process created by `fork()`.

---

## **How `fork()` Works**

### 1. **What Happens When `fork()` is Called?**
- The operating system creates a new process (the **child process**) that is a copy of the calling process (the **parent process**).
- Both processes will execute the same code starting from the point where `fork()` was called.

### 2. **Return Values of `fork()`**
- In the **parent process**, `fork()` returns the **PID (Process ID)** of the child process.
- In the **child process**, `fork()` returns **0**.
- If `fork()` fails (e.g., due to system resource limits), it returns **-1**, and no child process is created.

### 3. **Memory Duplication**
- The child process gets a copy of the parent process's memory.
- Modern operating systems use **Copy-On-Write (COW)**:
  - The parent and child initially share the same memory.
  - When either process modifies the memory, the OS creates a separate copy for the process making the modification.

---

## **Basic Example Using `fork()`**

```c
#include <stdio.h>
#include <unistd.h>

int main() {
    pid_t pid = fork(); // Create a new process

    if (pid == -1) {
        // Error: fork() failed
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Child process: PID = %d\n", getpid());
    } else {
        // Parent process
        printf("Parent process: PID = %d, Child PID = %d\n", getpid(), pid);
    }

    return 0;
}
```

### **Output Example**
```
Parent process: PID = 1234, Child PID = 1235
Child process: PID = 1235
```

---

## **Key Characteristics of `fork()`**

### 1. **Parent and Child are Separate Processes**
- Each process has its own:
  - Memory space.
  - File descriptors.
  - Execution context.

### 2. **Execution Order is Non-Deterministic**
- The parent and child processes execute independently.
- The OS scheduler determines the order of their execution.

### 3. **Resource Sharing**
- **File Descriptors**: Parent and child share open file descriptors, but changes (e.g., closing a file) in one process don't affect the other.
- **Memory**: Initially shared via Copy-On-Write.

---

## **Practical Scenarios for `fork()`**

1. **Parallel Execution**:
   - Running multiple processes simultaneously.
   - Example: Web servers handle multiple client requests using child processes.

2. **Inter-Process Communication (IPC)**:
   - Parent and child processes communicate using mechanisms like pipes, shared memory, or sockets.

3. **Process Isolation**:
   - Running independent tasks in isolated processes.

---

## **Advanced Example: Parent and Child Coordination**

This example demonstrates parent-child communication using `wait()` to ensure the parent waits for the child to finish.

```c
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // Child process
        printf("Child process: PID = %d\n", getpid());
        sleep(2); // Simulate work
        printf("Child process done.\n");
    } else {
        // Parent process
        printf("Parent process: PID = %d, waiting for child...\n", getpid());
        wait(NULL); // Wait for child to finish
        printf("Parent process: Child process finished.\n");
    }

    return 0;
}
```

### **Output Example**
```
Parent process: PID = 1234, waiting for child...
Child process: PID = 1235
Child process done.
Parent process: Child process finished.
```

---

## **Error Handling in `fork()`**

### Why `fork()` Might Fail
1. System resource limits (e.g., too many processes).
2. Memory allocation failure.

### Handling Failure
Check the return value of `fork()`:
```c
if (fork() == -1) {
    perror("fork failed");
    return 1;
}
```

---

## **Alternatives to `fork()`**

### 1. **`vfork()`**
- A lightweight version of `fork()` that doesn't copy the parent's memory space.
- The child runs in the parent's memory space until `exec()` or `_exit()` is called.

### 2. **`exec()` Family**
- Replaces the process image with a new program.
- Often used after `fork()` to run a different program in the child process.
- Example: `execl`, `execvp`, `execve`.

```c
pid_t pid = fork();
if (pid == 0) {
    execl("/bin/ls", "ls", NULL); // Replace child process with `ls`
    perror("execl failed");
    return 1;
}
```

---

## Exercises

Create a program which given natural number `n`:
1. the initial process creates `n` child processes
2. the initial process creates child process `p1`, `p1` creates its child process `p2`, ..., `p_{n -1}` creates `p_n`.
3. Where is the output of `execl`?

<details><summary>redirect to file</summary><code>
  pid_t pid = fork();
if (pid == 0) {
    int fd = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644); // Open a file for writing
    dup2(fd, STDOUT_FILENO);  // Redirect stdout to the file
    close(fd);                // Close the file descriptor (no longer needed)
    execl("/bin/ls", "ls", NULL);  // Replace process with `ls`
    perror("execl failed");
    return 1;
}
</code></details>

<details><summary>cascade</summary>
  <code>

    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/wait.h>
    
    void create_process_chain(int current, int max_depth);

    int main(int argc, char *argv[]) {
        if (argc != 2) {
            fprintf(stderr, "Usage: %s <number_of_processes>\n", argv[0]);
            return 1;
        }
    
        int n = atoi(argv[1]);
        if (n <= 0) {
            fprintf(stderr, "Number of processes must be positive\n");
            return 1;
        }
    
        printf("Process chain starting with main process (PID: %d)\n", getpid());
        
        // Start the process chain with the main process (process 0)
        create_process_chain(0, n);
        
        return 0;
        }
    
    void create_process_chain(int current, int max_depth) {
        if (current >= max_depth) {
            // We've reached the desired depth, no more processes to create
            printf("Process %d (PID: %d) - End of chain\n", current, getpid());
            return;
        }

        printf("Process %d (PID: %d) is creating process %d\n", 
               current, getpid(), current + 1);
    
        // Fork a child process
        pid_t pid = fork();
    
        if (pid < 0) {
            // Error
            perror("fork failed");
            exit(1);
        } else if (pid == 0) {
            // Child process
            printf("Process %d (PID: %d) was created by process %d\n", 
                   current + 1, getpid(), current);
            
            // Recursive call to create the next process in the chain
            create_process_chain(current + 1, max_depth);
            
            // Child exits after creating its child (if any)
            exit(0);
        } else {
            // Parent process
            // Wait for the child to complete its chain
            int status;
            waitpid(pid, &status, 0);
            printf("Process %d (PID: %d) - Child %d (PID: %d) has completed\n", 
                   current, getpid(), current + 1, pid);
        }
    }
</code></details>
