#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
using namespace std::chrono_literals;

struct X {
    X(const std::string& str) :object{ str } {}

    friend void swap(X& lhs, X& rhs);
private:
    std::string object;
    std::mutex m;
};

void swap(X& lhs, X& rhs) {
    if (&lhs == &rhs) return;
    std::lock(rhs.m, lhs.m);

    std::unique_lock<std::mutex> lock1{ lhs.m, std::adopt_lock };
    std::unique_lock<std::mutex> lock2{ rhs.m, std::adopt_lock };
    // std::lock(lock1, lock2);
    swap(lhs.object, rhs.object);
}

int main() {
    X a{ "🤣" }, b{ "😅" };
    std::thread t{ [&] {swap(a, b); } };  // 1
    std::thread t2{ [&] {swap(b, a); } }; // 2
    t.join();
    t2.join();
}