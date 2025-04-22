# Task 

The task involves workers producing and consuming `units` of a specific `type`. Implement this task using processes in c++. Below is the detailed explanation (beware that the following code is oversimplified and sometimes even pseudo-code is given):

---

## Concept

1. **Unit**:
   - Units are of `R` types, represented as non-negative integers.
   - The type of a unit can be checked using the function:
     ```cpp
     using unit = int;
     using unit_type = int;
      unit_type type(const unit& u) {
          return u; // Correct this - remember there are R types
      }
     ```

2. **Workers**:
   - Each worker has the following behavior:
     1. Produces a unit using the function:
        ```cpp
         auto produce() -> unit{
             static int counter = 0;
             u = counter++ % 5; // Produces units of 5 types (0 to 4) - make it random
             std::println("Produced: {}", u);
           return u;
         }
        ```
     2. Sends the unit to an intermediary process (`Helper`), which facilitates communication between workers.
     3. Receives a unit of the same type, produced by another worker, and consumes it using the function:
        ```cpp
         void consume(const unit& u) {
             std::println("Consumed: {}", u);
         }
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
        unit u = produce();
        send Helper[id].exchange(u);
        receive u;
        consume(u);
    }
}
```

---

