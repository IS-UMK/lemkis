# Hazard pointers

Hazard pointers are a lock-free memory management technique used in multithreaded environments to safely reclaim 
(roughly free <details>
  <summary>for details expand</summary>
  <p>Reclamation refers to the process of freeing or recovering resources, particularly memory, 
    that are no longer in use by a program or system. In computing, memory reclamation ensures efficient 
    utilization of finite memory by safely deallocating memory blocks that have been detached from data structures 
    but are still referenced by threads. This is critical in concurrent programming, where improper reclamation can 
    lead to issues such as use-after-free errors, memory leaks, or dangling pointers.
  </p>
</details>) memory in dynamic, 
lock-free data structures. They ensure that memory being accessed by one thread is not prematurely deallocated by another thread, 
addressing issues such as dangling pointers and the ABA problem.

## How Hazard Pointers Work
  1. Thread Ownership: Each thread maintains a small set of hazard pointers, which are thread-specific and represent memory locations the thread is currently accessing.
  2. Memory Protection: When a thread accesses a shared memory object, it sets a hazard pointer to point to that object. This signals other threads that the object cannot be reclaimed yet.
  3. Safe Reclamation: Before reclaiming memory, threads check all hazard pointers to ensure no other thread is referencing the memory. If no hazard pointer points to the memory, it is safe to deallocate.



