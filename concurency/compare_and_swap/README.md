# Some questions?

1. Does the processor executes the exact code we write? E.g. Loop unrolling, compile time optimizations, code removal, vectorized operations/registers etc.
2. Optimizations, branch prediction, caching, ooo execution (out of order execution e.g. while waiting for a file we can do sth else) etc.
3. ooo dynamic scheduling
  - instruction fetched
  - instruction dispatched to instruction queue
  - the instruction waits in the queue until its input operad is available 
  - if operands are available instruction the instruction is allowed to leave the queue before other instructions
  - the instruction is issued to to a functional unit
  - only if all older instructions have completed the operations result is written to register file

# Pure mutex
Reordering

Thread 1                  
flag1 = 1
if (flag2 == 0) { critical section}

Thread 2
flag2 = 1
if (flag1 == 0) { critical section}
