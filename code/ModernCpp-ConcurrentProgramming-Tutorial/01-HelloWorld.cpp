#include <iostream>
#include <thread>
#include <chrono>

void hello(){
    std::cout << "Hello World" << std::endl;
    // std::this_thread::sleep_for(std::chrono::seconds(5));
}

int main(){
    std::thread t; // 默认构造？构造不关联线程的 thread 对象
    std::cout <<std::boolalpha<< t.joinable() << '\n'; // false
}