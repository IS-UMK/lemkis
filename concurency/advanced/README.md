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

Unfortunately (as for now) no compiler supports <hazard_pointer> header yet. However to get a tasteof HPs:

```cpp
#include <atomic>
#include <stdexcept>
#include <iostream>
#include <thread>
#include <future>

// Node concept and definition
template <typename T>
concept Node = requires(T a) {
    { T::data };
    { *a.next } -> std::same_as<T&>;
};

template <typename T>
struct MyNode {
    T data;
    MyNode* next;
    MyNode(T d) : data(d), next(nullptr) {}
};

// Hazard Pointer Management
constexpr std::size_t MaxHazardPointers = 50;

template <typename T>
struct HazardPointer {
    std::atomic<std::thread::id> id;
    std::atomic<MyNode<T>*> pointer;
};

template <typename T>
HazardPointer<T> HazardPointers[MaxHazardPointers];

template <typename T>
std::atomic<MyNode<T>*>& getHazardPointer() {
    thread_local static HazardPointer<T> hazard;
    return hazard.pointer;
}

// Retire List for Memory Reclamation
template <typename T>
class RetireList {
    struct RetiredNode {
        MyNode<T>* node;
        RetiredNode* next;
        RetiredNode(MyNode<T>* p) : node(p), next(nullptr) {}
        ~RetiredNode() { delete node; }
    };

    std::atomic<RetiredNode*> retiredNodes;

public:
    void addNode(MyNode<T>* node) {
        auto retiredNode = new RetiredNode(node);
        retiredNode->next = retiredNodes.load();
        while (!retiredNodes.compare_exchange_strong(retiredNode->next, retiredNode));
    }

    void deleteUnusedNodes() {
        auto current = retiredNodes.exchange(nullptr);
        while (current) {
            auto next = current->next;
            delete current;
            current = next;
        }
    }
};

// Lock-Free Stack Implementation
template <typename T>
class LockFreeStack {
    std::atomic<MyNode<T>*> head;
    RetireList<T> retireList;

public:
    void push(T val) {
        auto newMyNode = new MyNode<T>(val);
        newMyNode->next = head.load();
        while (!head.compare_exchange_strong(newMyNode->next, newMyNode));
    }

    T topAndPop() {
        auto& hazardPointer = getHazardPointer<T>();
        MyNode<T>* oldHead = head.load();
        do {
            hazardPointer.store(oldHead);
            oldHead = head.load();
        } while (oldHead && !head.compare_exchange_strong(oldHead, oldHead->next));

        if (!oldHead) throw std::out_of_range("The stack is empty!");

        hazardPointer.store(nullptr);
        retireList.addNode(oldHead);
        retireList.deleteUnusedNodes();

        return oldHead->data;
    }
};

int main() {
    LockFreeStack<int> stack;

    stack.push(10);
    stack.push(20);

    std::cout << "Popped: " << stack.topAndPop() << '\n';
}

```

