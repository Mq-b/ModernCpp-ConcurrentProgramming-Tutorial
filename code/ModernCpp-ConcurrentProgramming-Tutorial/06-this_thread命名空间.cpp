#include <iostream>
#include <thread>
#include <chrono>
using namespace std::chrono_literals;

int main() {
    // 获取当前时间点
    auto now = std::chrono::system_clock::now();

    // 设置要等待的时间点为当前时间点之后的5秒
    auto wakeup_time = now + 5s;

    // 输出当前时间
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time:\t\t" << std::put_time(std::localtime(&now_time), "%H:%M:%S") << std::endl;

    // 输出等待的时间点
    auto wakeup_time_time = std::chrono::system_clock::to_time_t(wakeup_time);
    std::cout << "Waiting until:\t\t" << std::put_time(std::localtime(&wakeup_time_time), "%H:%M:%S") << std::endl;

    // 等待到指定的时间点
    std::this_thread::sleep_until(wakeup_time);

    // 输出等待结束后的时间
    now = std::chrono::system_clock::now();
    now_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "Time after waiting:\t" << std::put_time(std::localtime(&now_time), "%H:%M:%S") << std::endl;
}