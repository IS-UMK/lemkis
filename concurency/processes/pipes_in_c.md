# Pipes for Processes in C

**Pipes** in **C** are a mechanism for **Inter-Process Communication (IPC)**. They allow processes to exchange data by providing a unidirectional communication channel. Pipes are commonly used to transfer data between a parent process and its child process.

---

## Key Characteristics of Pipes

1. **Unidirectional**:
   - Data flows in one direction: from the write end to the read end.
   - If bidirectional communication is needed, two pipes are typically used.

2. **Parent-Child Relationship**:
   - Pipes are often used between a parent process and its child process.
   - The parent creates the pipe, then forks a child. Both processes can access the pipe.

3. **File Descriptor-Based**:
   - A pipe is represented by two file descriptors:
     - **Write end** (`pipe_fd[1]`): Used to write data into the pipe.
     - **Read end** (`pipe_fd[0]`): Used to read data from the pipe.

4. **Anonymous**:
   - Standard pipes are anonymous and only exist within the process that created them and its children. Named pipes (FIFOs) are a more advanced concept.

5. **Blocking Behavior**:
   - By default, reading from an empty pipe will block until data is available.
   - Writing to a full pipe will block until space becomes available.

---

## Creating a Pipe

To create a pipe in **C**, use the `pipe()` system call:

```c
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int pipe(int pipe_fd[2]);
```

- **Arguments**:
  - `pipe_fd`: An array of two integers that will store the file descriptors:
    - `pipe_fd[0]` for reading.
    - `pipe_fd[1]` for writing.

- **Return Value**:
  - `0` on success.
  - `-1` on failure (e.g., if system resources are insufficient).

---

## Basic Example

Here’s an example of using a pipe for communication between a parent and child process:

```c name=pipe_example.c
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int pipe_fd[2]; // Array to hold the pipe's file descriptors
    pid_t pid;

    // Create the pipe
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    // Fork a child process
    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(pipe_fd[1]); // Close unused write end

        char buffer[100];
        read(pipe_fd[0], buffer, sizeof(buffer));
        printf("Child received: %s\n", buffer);

        close(pipe_fd[0]); // Close read end
    } else {
        // Parent process
        close(pipe_fd[0]); // Close unused read end

        const char *message = "Hello from parent!";
        write(pipe_fd[1], message, strlen(message) + 1);

        close(pipe_fd[1]); // Close write end
        wait(NULL); // Wait for child to finish
    }

    return 0;
}
```

---

## Explanation of the Example

1. **Pipe Creation**:
   - The `pipe()` function creates a communication channel between processes:
     - `pipe_fd[0]`: Read end of the pipe.
     - `pipe_fd[1]`: Write end of the pipe.

2. **Forking a Child Process**:
   - The `fork()` system call creates a new process:
     - The child inherits the pipe file descriptors.

3. **Parent Process**:
   - Closes the **read end** (`pipe_fd[0]`) since it only writes to the pipe.
   - Writes a string to the pipe using `write()`.

4. **Child Process**:
   - Closes the **write end** (`pipe_fd[1]`) since it only reads from the pipe.
   - Reads data from the pipe using `read()`.

5. **Synchronization**:
   - The parent waits for the child to finish using `wait()`.

---

## Output

```
Child received: Hello from parent!
```

---

## Advanced Use Cases

1. **Bidirectional Communication**:
   - Use two pipes: one for each direction of communication.

2. **Named Pipes (FIFOs)**:
   - Allow communication between unrelated processes.
   - Created using `mkfifo()`.

3. **Non-Blocking I/O**:
   - Use `fcntl()` to set the pipe file descriptors to non-blocking mode.

4. **Pipes with `dup2()`**:
   - Redirect standard input/output to a pipe for more advanced scenarios (e.g., chaining commands).

---

## Limitations of Pipes

1. **Unidirectional**:
   - Standard pipes only support one-way communication.

2. **Parent-Child Scope**:
   - Pipes are limited to processes with a parent-child relationship unless named pipes are used.

3. **Buffer Size**:
   - Pipes have a fixed buffer size. Writing more data than the buffer size will block the process.

---

## Summary

Pipes in C are a powerful IPC mechanism, especially for parent-child communication. They are lightweight, easy to use, and sufficient for many basic use cases. For more complex IPC requirements (e.g., bidirectional communication or communication between unrelated processes), named pipes, sockets, or shared memory may be better alternatives.
