#include <chrono>
#include <iostream>
#include <iomanip>
#include <future>
using namespace std::chrono_literals;

int main(){
    using namespace std::chrono;
    auto future = std::async(std::launch::deferred, []{
        std::cout << "deferred\n";
    });

    if (future.wait_for(35ms) == std::future_status::deferred)
        std::cout << "future_status::deferred " << "正在延迟执行\n";

    future.wait(); // 在 wait() 或 get() 调用时执行，不创建线程
}