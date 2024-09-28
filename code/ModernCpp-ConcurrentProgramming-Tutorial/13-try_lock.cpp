#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
using namespace std::string_literals;

std::mutex mtx;

void thread_function(int id) {
    // 尝试加锁
    if (mtx.try_lock()) {
        std::string s = "线程："s + std::to_string(id) + " 获得锁"s + "\n";
        std::string s2 = "线程："s + std::to_string(id) + " 释放锁"s + "\n";
        std::cout << s;
        // 临界区代码
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 模拟临界区操作
        mtx.unlock(); // 解锁
        std::cout << s2;
    }
    else {
        std::string s = "线程："s + std::to_string(id) + " 获取锁失败 处理步骤"s + "\n";
        std::cout << s;
    }
}

int main(){
    std::thread t1(thread_function, 1);
    std::thread t2(thread_function, 2);

    t1.join();
    t2.join();
}