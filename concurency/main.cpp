#include <bank_account.hpp>
#include <cas.hpp>
#include <condition_variable.hpp>
#include <print>


auto double_inc_example(std::atomic<double> &x) {
    std::jthread t1{[&x]() { atomic_multiply_example(x, 2.0); }};
    std::jthread t2{[&x]() { atomic_multiply_example(x, 3.0); }};
    std::jthread t3{[&x]() { atomic_multiply_example(x, 4.0); }};
}

auto stack_push_example(stack<int> &s) {
    std::jthread t1{[&s]() { s.push(1); }};
    std::jthread t2{[&s]() { s.push(2); }};
    std::jthread t3{[&s]() { s.push(3); }};
}

auto factory_example(factory &f) {
    std::jthread t2{[&f]() {
        while (true) { f.consume_a_cookie(); }
    }};
    std::jthread t3{[&f]() {
        while (true) { f.consume_a_cookie(); }
    }};
    std::jthread t1{[&f]() {
        while (true) { f.produce_a_cookie(); }
    }};
}


int main() {

    deadlock_example();
    safe_example();
    std::atomic<double> x{1.0};
    double_inc_example(x);
    std::println("x after double example = {}", x.load());
    stack<int> s{};
    for (int i{0}; i < 10; i++) { stack_push_example(s); }
    auto head{s.head.load()};
    int i{0};
    while (head) {
        std::print("{}{}", (((i % 3) == 0) ? '\n' : ','), head->data);
        head = head->next;
        ++i;
    }
    std::println("\n\n\n Now comes condition variable example:\n\n\n");
    worker w;
    w.main_thread();
    factory f{};
    factory_example(f);
    return 0;
}