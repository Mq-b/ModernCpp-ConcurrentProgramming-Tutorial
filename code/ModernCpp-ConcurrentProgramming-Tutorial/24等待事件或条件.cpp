#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
using namespace std::chrono_literals;

std::mutex mtx; // 互斥量
std::condition_variable_any cv; // 条件变量
bool arrived = false;

void wait_for_arrival() {
    std::unique_lock<std::mutex> lck(mtx); // 上锁
    cv.wait(lck, [] { return arrived; }); // 等待 arrived 变为 true 会解锁的 再次上锁
    std::cout << "到达目的地，可以下车了！" << std::endl;
}

void simulate_arrival() {
    std::this_thread::sleep_for(std::chrono::seconds(5)); // 模拟地铁到站，假设5秒后到达目的地
    {
        std::lock_guard<std::mutex> lck(mtx);
        arrived = true; // 设置条件变量为 true，表示到达目的地
    }
    cv.notify_one(); // 通知等待的线程
}

int main(){
    std::thread t{ wait_for_arrival };
    std::thread t2{ simulate_arrival };
    t.join();
    t2.join();
}