#include <iostream>
#include <thread>
#include <future> // 引入 future 头文件

void f() {
    std::cout << std::this_thread::get_id() << '\n';
}

int main() {
    auto t = std::async([] {});
    std::future<void> future{ std::move(t) };
    future.wait();   // Error! 抛出异常
}