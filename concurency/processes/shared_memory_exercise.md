# Concurrent Programming
## Exercises 1 – The Mutual Exclusion Problem

### Task 1

We concurrently start the following two processes:

```cpp
int y = 0;

process P1() {
    int x, i;
    for (i = 1; i < 6; i++) {
        x = y;
        x = x + 1;
        y = x;
    }
}

process P2() {
    int x, i;
    for (i = 1; i < 6; i++) {
        x = y;
        x = x + 1;
        y = x;
    }
}

```

What value will the variable `y` have after both processes have finished executing?  
Assume that a variable declared outside the process body is a global variable accessible by all processes.

---

### Task 2

We concurrently start the following two processes:


```cpp
process P1() {
    while (true) {
        local_section;
        entry_protocol;
        critical_section;
        exit_protocol;
    }
}

process P2() {
    while (true) {
        local_section;
        entry_protocol;
        critical_section;
        exit_protocol;
    }
}
```

We want to ensure that at most one of them executes the part of the program marked as `critical_section` at any given time.  
What instructions should be placed in the protocols to achieve this goal?  
Assume that we do not have any synchronization mechanisms available, so the protocols should skillfully use global variables and programming language instructions.

- <details>
  <summary>Can the entry and exit protocols be left empty?</summary>
  <p>No, the entry and exit protocols <strong>cannot be left empty</strong> if you want to guarantee mutual exclusion.</p>
  <p>The <em>entry protocol</em> ensures that a process waits until it is safe to enter the critical section (i.e., no other process is inside).</p>
  <p>The <em>exit protocol</em> releases any locks or signals that allow other waiting processes to enter the critical section.</p>
  <p>Without these protocols, multiple processes could enter the critical section simultaneously, violating the mutual exclusion property.</p>
  <p>Thus, empty protocols mean no control over access, which breaks the mutual exclusion requirement.</p>
</details>

- <details>
  <summary>What does safety mean in the context of this problem?</summary>
  <p><strong>Safety</strong> means that the mutual exclusion property is preserved:</p>
  <ul>
    <li>At most one process can be executing in the critical section at any given time.</li>
    <li>No two or more processes are allowed to be inside their critical sections simultaneously.</li>
  </ul>
  <p>This property prevents race conditions and inconsistent shared state.</p>
</details>

- <details>
  <summary>What does liveness mean?</summary>
  <p><strong>Liveness</strong> means that the system guarantees progress:</p>
  <ul>
    <li>If a process wants to enter the critical section, it will eventually be allowed to do so (no indefinite postponement or starvation).</li>
    <li>The system avoids deadlocks (where processes wait forever) and starvation (where some processes are perpetually denied access).</li>
    <li>Every request to enter the critical section eventually succeeds.</li>
  </ul>
</details>


In the solution, we will use global and local variables.  
A local variable may reside in the private address space of a process. Other processes do not have access to it and cannot read or modify it.  
The situation is different with global variables: they are shared between processes, which means that at any moment any process can modify or read them.

- What happens when two concurrently executing processes try to access the same variable, i.e., the same memory cell(s), at the same time?
    - The conflict is resolved by hardware through a memory arbiter. This is a hardware unit that ensures mutual exclusion when accessing individual memory cells. Simultaneous accesses to the same memory cell will be ordered in time in some unpredictable way and executed. In further considerations, we assume the existence of a memory arbiter and its correct operation.

---

## First Attempt at a Solution

Let’s first try to solve the problem by introducing a global variable `whoseturn`.  
It will take values 1 or 2.  
Value 2 means that process one must wait, and process two has the right to enter the critical section.

Waiting for entry is implemented by an empty loop whose only task is to repeatedly check the condition for entering the critical section.  
A process that uses the critical section passes priority to its competitor.


```cpp
int whoseturn = 1;

process P1() {
    while (true) {
        local_section;
        while (whoseturn == 2) { }
        critical_section;
        whoseturn = 2;
    }
}

process P2() {
    while (true) {
        local_section;
        while (whoseturn == 1) { }
        critical_section;
        whoseturn = 1;
    }
}

```


Before analyzing the correctness of the solution, let’s recall the assumptions concerning the critical section, local section, and the operating system supervising process execution.

Decide which of the following statements are true:

- When formulating the mutual exclusion problem, it is assumed that:
    - Every process that enters the critical section will leave it in finite time.
    - A process cannot stay in the critical section longer than a predetermined time.
    - A process cannot terminate in the critical section.
    - A process can terminate in the critical section, but only in a correct manner.
- A process that has started executing the local section:
    - Must finish it in finite time and proceed to the entry protocol.
    - Can terminate due to an error.
    - Can terminate, but only in a correct manner.

Given these assumptions, is the proposed solution correct?  
**Safety** property is maintained-both processes will never be in the critical section simultaneously.  
But what about **liveness**?  
Recall the assumption that a process does not stay indefinitely in the critical region.  
Therefore, after leaving it (which will eventually happen), the local section is executed.  
Here lies the problem-nothing can be assumed about this part of the program.  
If a process gets stuck here (due to an error, infinite loop, etc.), the other process will be able to enter the critical section at most one more time.  
Subsequent attempts will result in the process being blocked "forever."  
Thus, the presented solution lacks the liveness property.  
Another disadvantage is the overly tight coupling between the processes.  
They must use the critical section alternately, but their needs may differ.  
If, in addition, they operate at different "speeds" (e.g., one’s local section is more complex), the "faster" process will be forced to match the pace of the slower one.

---

## Second Attempt at a Solution

Let’s try a different approach.  
Introduce two global logical variables:  
- `in1` indicating that process P1 is in the critical section,
- `in2` indicating that process P2 is in the critical section.

Before entering the critical section, a process checks if its rival is already in the critical section. If so, it waits.  
When the critical section becomes free, the process sets its variable to true, signaling that it is in the critical section, and then enters it.


```cpp
bool in1 = false;
bool in2 = false;

process P1() {
    while (true) {
        local_section;
        while (in2) { }
        in1 = true;
        critical_section;
        in1 = false;
    }
}

process P2() {
    while (true) {
        local_section;
        while (in1) { }
        in2 = true;
        critical_section;
        in2 = false;
    }
}
```

Notice that this solution no longer tightly couples the processes.  
If one of them does not want to use the critical section or terminates in the local section, the other can freely enter the critical section as many times as it wants.  
Unfortunately, with certain malicious interleavings, both processes can enter the critical section simultaneously.  
Indicate such a scenario.  
The cause is that the logical variables are set too late.  
A process is "almost" in the critical section (having passed the waiting loop) but has not yet informed its rival (i.e., has not set its `in` variable).

Since there exists an interleaving that leads to an incorrect situation, by the definition of correctness (it must be "good" for every interleaving), we conclude that the above program is incorrect.  
However, if checking the condition in the loop and changing the variable value were atomic, such a bad scenario would not be possible.  
But guaranteeing atomicity would mean creating a critical section, which is the very problem we are trying to solve.

---

## Third Attempt at a Solution

Since changing the value of global variables in the previous solution happened too late, let’s try changing the order of instructions:  
First set the logical variable, then try to enter the loop.  
Now the logical variables indicate the intention to enter the critical section.



```cpp
bool want1 = false;
bool want2 = false;

process P1() {
    while (true) {
        local_section;
        want1 = true;
        while (want2) { }
        critical_section;
        want1 = false;
    }
}

process P2() {
    while (true) {
        local_section;
        want2 = true;
        while (want1) { }
        critical_section;
        want2 = false;
    }
}
```

Now we have a safe program.  
Indeed, at most one process can be in the critical region at a time.  
But liveness is missing!  
It is easy to cause a deadlock.  
Indicate an appropriate scenario.

---

## Fourth Attempt at a Solution

One can try to remedy the situation by forcing the processes to temporarily give up entry to the section and yield priority.


```cpp
bool want1 = false;
bool want2 = false;

process P1() {
    while (true) {
        local_section;
        want1 = true;
        while (want2) {
            want1 = false;
            want1 = true;
        }
        critical_section;
        want1 = false;
    }
}

process P2() {
    while (true) {
        local_section;
        want2 = true;
        while (want1) {
            want2 = false;
            want2 = true;
        }
        critical_section;
        want2 = false;
    }
}
```


Unfortunately, there still exists a malicious interleaving that causes a lack of liveness.  
It does not help to argue that such an interleaving is highly unlikely.  
According to the definition of correctness, since there is a scenario causing a lack of liveness, the program is incorrect.

---

## Peterson's Algorithm

A correct solution, known as **Peterson's algorithm**, combines the first idea with the penultimate one.  
We maintain the variables `want1` and `want2`, which indicate a process's desire to enter the critical section.  
If both processes want to enter the critical section, the conflict is resolved using the variable `whoseturn`.



```cpp
bool want1 = false;
bool want2 = false;
int whoseturn = 1;

process P1() {
    while (true) {
        local_section;
        want1 = true;
        whoseturn = 1;
        while (want2 && (whoseturn == 1)) { }
        critical_section;
        want1 = false;
    }
}

process P2() {
    while (true) {
        local_section;
        want2 = true;
        whoseturn = 2;
        while (want1 && (whoseturn == 2)) { }
        critical_section;
        want2 = false;
    }
}
```


Remember, we do not assume atomicity (indivisibility) of individual instructions (in particular, evaluating complex logical conditions need not be atomic).  
We do, however, assume the existence of a memory arbiter (simultaneous attempts to change the value of `whoseturn` may occur).

Notice that if only one process is competing for access to the critical section, it will be able to use it as much as it wants, since the rival's `want` variable will always be `false`.  
This avoids the tight coupling of processes seen in the first solution.  
On the other hand, if both processes always want to use the critical section, they will take turns.

---

### Algorithm Analysis

Analyze Peterson's algorithm with respect to the order of instruction execution:

- Can the order of assignments before the `while` loop be changed?
- Can the change of the `whoseturn` variable be placed after leaving the critical section?
- Can the order of checking conditions in the `while` loop be changed?
- Can the `whoseturn` variable be initialized with other values?
- What about the `want1` and `want2` variables?

**Disadvantages of Peterson's Algorithm:**

- **Busy waiting.** Since no process suspension mechanisms were available, the only option was to stop the process in an idle loop. However, this uses CPU time (and the data bus). In the future, busy waiting will be considered a serious flaw.
- **The number of processes must be known in advance.**
- **The cost of the entry protocol is significant.**

---
