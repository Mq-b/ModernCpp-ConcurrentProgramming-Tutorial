#include <iostream>
#include <thread>
#include <chrono>
#include <latch>
using namespace std::chrono_literals;

std::latch latch{ 10 };

void f(int id) {
    //todo.. 脑补任务
    std::cout << std::format("线程 {} 执行完任务，开始等待其它线程执行到此处\n", id);
    latch.arrive_and_wait(); // 减少 并等待  count_down(1); wait(); 等待计数为 0
    std::cout << std::format("线程 {} 彻底退出函数\n", id);
}

int main() {
    std::vector<std::jthread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(f, i);
    }
}