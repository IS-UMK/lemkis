# Parallel Programming
## Basic Concepts

### Definition: Process
A **process** is an instance of a program in execution. It serves as a container for resources such as memory, file handles, and system resources, and operates independently from other processes. A process contains:
- **Code Segment**: The program's executable instructions.
- **Data Segment**: Global variables and static data used by the program.
- **Heap**: Dynamically allocated memory during runtime.
- **Threads**: One or more threads that execute tasks within the process.

Processes do not share memory directly with other processes; instead, they communicate through inter-process communication (IPC) mechanisms such as pipes, message queues, or shared memory.

### Definition: Thread
A **thread** is the smallest unit of execution within a process. It consists of a sequence of instructions that can be executed independently by the CPU. Threads within the same process share the same memory space and resources, allowing for efficient communication and data sharing. Each thread has its own execution context, which includes:
- **Program Counter (PC)**: Tracks the next instruction to execute.
- **Stack**: Stores local variables and function call information.
- **Registers**: Used for temporary storage during execution.

Threads enable concurrent execution of tasks within a single process.

### Definition: Scheduler
A **scheduler** is a component of the operating system responsible for managing the execution of threads and processes. Its main responsibilities include:
- **Context Switching**: Saving the state of a currently running thread or process and restoring the state of another to enable multitasking.
- **Resource Allocation**: Assigning CPU time, memory, and other resources to threads or processes.
- **Execution Order**: Determining which thread or process runs at any given time based on scheduling algorithms.

The scheduler ensures efficient utilization of system resources and may use algorithms such as First-Come-First-Served (FCFS), Round Robin, or Priority Scheduling to manage execution order.

## Related Concepts

To fully understand threads, processes, and schedulers, it is important to clarify additional terms:

### Definition: Concurrency
Concurrency refers to the ability of a system to manage multiple tasks simultaneously. While tasks may not execute at the exact same time (as in parallelism), their execution overlaps by interleaving operations. Concurrency allows programs to remain responsive by performing multiple operations seemingly at once.

### Definition: Parallelism
Parallelism is a subset of concurrency where multiple tasks are executed simultaneously on separate processors or cores. It is often used to divide computational workloads across multiple execution units to improve performance.

### Definition: Context Switching
Context switching is the process by which the CPU switches from executing one thread or process to another. This involves saving the current state (e.g., program counter, registers) of the running thread or process and restoring the state of another. Context switching enables multitasking but introduces overhead due to saving and restoring states.

### Definition: Race Condition
A race condition occurs in concurrent programming when two or more threads access shared data simultaneously and at least one modifies it without proper synchronization. This can lead to unpredictable behavior and incorrect results.

### Definition: Deadlock
A deadlock is a state in concurrent programming where two or more threads are blocked indefinitely because each is waiting for a resource held by another thread. Deadlocks occur when there is circular dependency among threads for acquiring resources.

## Summary

In summary:
- A thread is an independent unit of execution within a process that shares memory with other threads in the same process.
- A scheduler manages how threads and processes are executed on the CPU using scheduling algorithms.
- A process is an instance of a program in execution that contains its own memory space and resources but may have multiple threads for concurrent task execution.
- Concurrency enables overlapping task execution, while parallelism focuses on simultaneous task execution on multiple cores or processors.
- Context switching facilitates multitasking but introduces overhead, while race conditions and deadlocks are challenges that must be addressed in concurrent programming.

These definitions provide precise explanations of key concepts in concurrent programming.
