#include <chrono>
#include <iostream>
#include <iomanip>
using namespace std::chrono_literals;

int main(){
    auto now = std::chrono::system_clock::now();
    time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time:\t" << std::put_time(std::localtime(&now_time), "%H:%M:%S\n");

    auto now2 = std::chrono::steady_clock::now();
    now_time = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time:\t" << std::put_time(std::localtime(&now_time), "%H:%M:%S\n");
}