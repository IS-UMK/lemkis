#include "bank_account.hpp"
// Example of deadlock
/* void deadlock_example() {
    BankAccount account1(1000);
    BankAccount account2(1000);
    
    std::print("Demonstrating deadlock problem:\n");
    
    std::thread t1([&account1, &account2]() {
        account1.transfer(account2, 500);
    });
    
    std::thread t2([&account1, &account2]() {
        account2.transfer(account1, 300);
    });
    
    t1.join();
    t2.join();
} */



int main() {
    return 0;
}