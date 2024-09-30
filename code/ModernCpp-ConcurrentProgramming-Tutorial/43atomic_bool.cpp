#include <atomic>
#include <iostream>
#include <thread>

std::atomic<bool> flag{ false };
bool expected = false;

void try_set_flag() {
    // 尝试将 flag 设置为 true，如果当前值为 false
    if (flag.compare_exchange_strong(expected, true)) {
        std::cout << "flag 为 false，flag 设为 true。\n";
    }
    else {
        std::cout << "flag 为 true, expected 设为 true。\n";
    }
}

int main() {
    std::thread t1{ try_set_flag };
    std::thread t2{ try_set_flag };
    t1.join();
    t2.join();
    std::cout << "flag: " << std::boolalpha << flag << '\n';
    std::cout << "expected: " << std::boolalpha << expected << '\n';
}