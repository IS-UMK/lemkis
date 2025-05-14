The task is to write example implementation of the following problem descriptions in c++ using processes (remember about technical aspects like a synchronization, 
correctness of the solution and style requirements).

#  Dates

In the system, there are `N` pairs of processes. 
The processes in each pair are indistinguishable. 
Each process cyclically performs its own `tasks`, and then meets with the other process from the pair in a café at a two-person table (function `date()`). 
There is only one table in the café. Processes occupy the table only when both are ready to meet, but they may leave it individually, that is, 
they can finish executing the `date()` function at different times.

# Group Synchronization

In the system, there are `N` groups of processes. 
Each process cyclically executes its own procedure (local section), and then the function  `compute()`, 
which at the same time can only be executed by processes belonging to the same group. 
The first process in a group can start executing the `compute()` function only when no one else is executing it. 
After finishing the execution of the `compute()` procedure, 
processes wait until all processes executing it have completed. 
After the last process executing the procedure finishes, the processes from the next group waiting to execute can begin their operation.



