# Shared pointers

## std::shared_ptr

**std::shared_ptr** is a smart pointer that retains shared ownership of an object through a pointer. Several shared_ptr objects may own the same object. The object is destroyed and its memory deallocated when either of the following happens:
  - the last remaining shared_ptr owning the object is destroyed;
  - the last remaining shared_ptr owning the object is assigned another pointer via operator= or reset().

All member functions (including copy constructor and copy assignment) can be called by multiple threads on different shared_ptr objects without additional synchronization even if these objects are copies and share ownership of the same object. If multiple threads of execution access the same shared_ptr object without synchronization and any of those accesses uses a non-const member function of shared_ptr then a data race will occur; the std::atomic<shared_ptr> can be used to prevent the data race.

## std::atomic&lang;std::shared_ptr&rang;

If multiple threads of execution access the same std::shared_ptr object without synchronization and any of those accesses uses a non-const member function of shared_ptr then a data race will occur unless all such access is performed through an instance of std::atomic<std::shared_ptr>

Associated `use_count` increments are guaranteed to be part of the atomic operation. Associated `use_count` decrements are sequenced after the atomic operation, but are not required to be part of it, except for the `use_count` change when overriding `expected` in a failed CAS. Any associated deletion and deallocation are sequenced after the atomic update step and are not part of the atomic operation.
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

## Lock free stack example

Unfortunately (as for now) no compiler supports <hazard_pointer> header yet. However to get a tasteof HPs we provide some examples. To understand what is going on let us explain two concepts:
1. **retired object** refers to an object that is no longer part of an active data structure but cannot be immediately deallocated due to potential ongoing access by other threads. The retirement process ensures that the object is safely removed from the structure while deferring its reclamation until it is confirmed that no hazard pointers are referencing it.
2. **std::hazard_pointer_domain** is a core component of the C++26 Hazard Pointers API that manages groups of hazard pointers and retired objects for safe memory reclamation in concurrent programs. It acts as a synchronization boundary for memory reclamation operations.
   1.  Objects retired to one domain are only checked against hazard pointers within that domain.
   2.  Multiple domains: Allow separate lock-free data structures to operate independently without cross-contamination.
   3.  Owns all hazard pointers created for its domain.
   4.  Manages retired objects queued for deferred reclamation.


```cpp
#include <hazard_pointer>
#include <atomic>
#include <iostream>
#include <memory>

struct Node : std::hazard_pointer_obj_base<Node> {
    int data;
    Node* next;
    
    Node(int val) : data(val), next(nullptr) {}
};

class LockFreeStack {
    std::atomic<Node*> head{nullptr};
    std::hazard_pointer_domain domain; // Hazard pointer domain

public:
    void push(int value) {
        auto* newNode = new Node(value);
        newNode->next = head.load();
        while (!head.compare_exchange_strong(newNode->next, newNode));
    }

    std::unique_ptr<int> pop() {
        std::hazard_pointer hp(domain); // RAII hazard pointer
        
        Node* oldHead;
        do {
            oldHead = head.load();
            hp.protect(oldHead); // Protect the node
        } while (oldHead && !head.compare_exchange_strong(oldHead, oldHead->next));

        if (!oldHead) return nullptr;
        
        auto result = std::make_unique<int>(oldHead->data);
        oldHead->retire(); // Schedule for safe reclamation
        return result;
    }

    ~LockFreeStack() {
        domain.reclaim(); // Force cleanup of all retired nodes
    }
};

int main() {
    LockFreeStack stack;
    stack.push(42);
    stack.push(100);

    auto val1 = stack.pop();
    auto val2 = stack.pop();
    
    if (val1) std::cout << "Popped: " << *val1 << "\n";
    if (val2) std::cout << "Popped: " << *val2 << "\n";
}
```

or 

```cpp
#include <hazard_pointer>
#include <vector>
#include <iostream>

struct GraphNode : std::hazard_pointer_obj_base<GraphNode> {
    int id;
    std::vector<GraphNode*> neighbors;
    
    GraphNode(int id) : id(id) {}
};

void safe_traverse(GraphNode* start) {
    std::hazard_pointer_domain domain;
    std::hazard_pointer hp(domain);
    
    hp.protect(start); // Protect root node
    for (auto neighbor : start->neighbors) {
        hp.protect(neighbor); // Protect each neighbor
        std::cout << "Traversing edge: " << start->id 
                  << " -> " << neighbor->id << "\n";
        
        // Process neighbor (protected by hazard pointer)
    }
    
    // Automatic cleanup via RAII
}
```
