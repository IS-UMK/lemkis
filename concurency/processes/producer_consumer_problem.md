# Task 

The task involves workers producing and consuming units of a specific type. Below is the detailed explanation:

---

## Concept

1. **Unit**:
   - Units are of `R` types, represented as non-negative integers.
   - The type of a unit can be checked using the function:
     ```cpp
     int get_type(unit u);
     ```

2. **Workers**:
   - Each worker has the following behavior:
     1. Produces a unit using the function:
        ```cpp
        void produce(unit &u);
        ```
     2. Sends the unit to an intermediary process (`Helper`), which facilitates communication between workers.
     3. Receives a unit of the same type, produced by another worker, and consumes it using the function:
        ```cpp
        void consume(unit u);
        ```

3. **Assumptions**:
   - Each worker produces an infinite number of units of all types.
   - Units are passed between workers through helper processes (`Helper`).

4. **Helper Processes**:
   - Helpers are implemented as an `N`-element array.
   - Each `Helper[i]` can only:
     - Send messages to its associated worker.
     - Send messages to the next helper in a circular manner, i.e., `Helper[(i + 1) % N]`.

---

## Goal

The goal is to implement the `Helper` processes, assuming that workers follow this algorithm:

```plaintext
process Worker[id : 1..N] {
    while (true) {
        unit u;
        produce(u);
        send Helper[id].exchange(u);
        receive u;
        consume(u);
    }
}
```

---

### Summary

This task involves creating processes (`Helpers`) to facilitate the exchange of units between workers, ensuring that each worker receives and consumes a unit of the same type as the one it produced.
