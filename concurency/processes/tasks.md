The tasks are to write example implementations of the following problem descriptions in c++ using processes (remember about technical aspects like a synchronization, 
correctness of the solution and style requirements). Each of you during your presentation is obliged to
- smoothly explain what the problem is and what are the challenges
- explain how you solved it (give the solution idea)
- present the implementation code
- test the code with in regular and extreme cases

Moreover, each solution must be a cmake project which is linked to the root CMakeLists.txt (use `add_subdirectory` in proper places). Moreover, github actions must pass (that is clang-format and clang-tidy do not throw any warnings/errors)
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
process clerk(int rank, int id) {
    while (true) {
        office::want_to_work(rank, id);
        // talking with a client
        office::finished();
        // resting
    }
}

process client() {
    int c1, c2;
    while (true) {
        office::want_to_handle_issue(c1, c2);
        // talking with clerks c1 and c2
        office::handled();
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
- The office should operate in such a way as to serve the greatest possible number of clients, and **this is the priority in this task**.

# Printers 

In an office building, there are `N` companies. There are also $`0 < K \le N`$ printers, which are shared by the employees of these companies. The rules for using the printers are as follows:

Employees of different companies never use the same printer simultaneously (we assume there is a procedure `print(int printer_number)`, whose invocation means using the printer: sending a document, waiting for printing, and collecting the document from the printer). An employee of a given company cannot start printing unless they are certain that the printer to which they want to send their document is not being used by employees of other companies. Employees of the same company can use only one printer at a time. If there are many free printers, an employee whose company is not currently printing can choose any of them (we do not care about equal distribution of printer usage).



# Basketball 

There are $`N > 0`$ teams using a basketball court. Each team consists of $`K > 1`$ players, and each player is permanently assigned to a specific team.

Upon arriving at the court, a player waits for all players from their team to arrive as well as for another complete team to form. A match can start only when there are two complete teams and the court is free. Each team must learn the number of the opposing team: each player calls the procedure `match(int opponent)`.

A player can leave the court at any time (ending the execution of the `match` procedure). The remaining players continue the game, even if only one player remains on the court. A player who has finished the game cannot return to it; to start a new match, they must first wait until all players of their team and some opposing team gather again.

Matches are played in a sports center with $$B$$ courts numbered from 0 to $$B - 1$$.

