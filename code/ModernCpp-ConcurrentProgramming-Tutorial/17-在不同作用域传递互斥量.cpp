#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>

std::unique_lock<std::mutex> get_lock() {
    extern std::mutex some_mutex;
    std::unique_lock<std::mutex> lk{ some_mutex };
    return lk; // 选择到 unique_lock 的移动构造，转移所有权
}
void process_data() {
    std::unique_lock<std::mutex> lk{ get_lock() }; // 转移到了主函数的 lk 中
    // 执行一些任务...
}// 最后才会 unlock 解锁

int main(){
    process_data();
}