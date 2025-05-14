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

# Office

In a certain office, a group of clerks serves a group of clients. Each clerk has a unique identifier. The algorithms for clerks and clients are as follows:
```cpp
process Clerk(int rank, int id) {
    while (true) {
        Office.WantToWork(rank, id);
        // talking with a client
        Office.Finished;
        // resting
    }
}

process Client() {
    int c1, c2;
    while (true) {
        Office.WantToHandleIssue(&c1, &c2);
        // talking with clerks c1 and c2
        Office.Handled;
        // own matters
    }
}
```


- In the office, there is at least one clerk of higher rank or at least two clerks of lower rank.
- The office is equipped with $$K$$ chairs ($$K > 2$$), which are used by clerks and clients during conversations.
- A conversation takes place if there is an interested client and either a willing clerk of higher rank or two clerks of lower rank, and there are enough chairs available (each person occupies one chair).
- Clerks within each group work according to the order in which they signal their availability to work.
- Clients must find out which clerks they will talk to (i.e., they must learn their identifiers).
- After the conversation is finished, clerks and clients may leave the chairs individually.
- The office should operate in such a way that...

