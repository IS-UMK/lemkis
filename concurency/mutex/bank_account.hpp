#pragma once

#include <print>


#include <mutex>
#include <thread>
#include <print>

class BankAccount {
private:
    double balance;
    std::mutex mtx;

public:
    BankAccount(double initial_balance) : balance(initial_balance) {}

    void withdraw(double amount) {
        std::lock_guard<std::mutex> lock(mtx);
        if (balance >= amount) {
            // Simulate some processing time
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            balance -= amount;
            std::print("Withdrew {}, new balance: {}\n", amount, balance);
        } else {
            std::print("Insufficient funds\n");
        }
    }

    void transfer(BankAccount& to, double amount) {
        // DEADLOCK DANGER: If two threads call transfer in opposite directions
        // Thread 1: account1.transfer(account2, amount)
        // Thread 2: account2.transfer(account1, amount)
        // They will deadlock because each thread holds one mutex and waits for the other
        std::lock_guard<std::mutex> lock_from(mtx);
        std::print("Locked source account\n");
        
        // Simulate some processing time before acquiring the second lock
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::lock_guard<std::mutex> lock_to(to.mtx);
        std::print("Locked destination account\n");
        
        if (balance >= amount) {
            balance -= amount;
            to.balance += amount;
            std::print("Transferred {} successfully\n", amount);
        } else {
            std::print("Transfer failed due to insufficient funds\n");
        }
    }

    // A safer way to transfer that prevents deadlock using std::scoped_lock (C++17)
    void safe_transfer(BankAccount& to, double amount) {
        // std::scoped_lock automatically locks all mutexes without risk of deadlock
        // and unlocks them when it goes out of scope
        std::scoped_lock lock(mtx, to.mtx);
        
        if (balance >= amount) {
            balance -= amount;
            to.balance += amount;
            std::print("Safely transferred {} successfully\n", amount);
        } else {
            std::print("Safe transfer failed due to insufficient funds\n");
        }
    }

    double get_balance() {
        std::lock_guard<std::mutex> lock(mtx);
        return balance;
    }
};

// Example of deadlock
void deadlock_example() {
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
}


void safe_example() {
    BankAccount account1(1000);
    BankAccount account2(1000);
    
    std::print("Demonstrating deadlock prevention with std::scoped_lock:\n");
    
    std::thread t1([&account1, &account2]() {
        account1.safe_transfer(account2, 500);
    });
    
    std::thread t2([&account1, &account2]() {
        account2.safe_transfer(account1, 300);
    });
    
    t1.join();
    t2.join();
}
