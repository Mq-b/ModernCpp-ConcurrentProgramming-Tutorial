#include <iostream>
#include <thread>
#include <future>

std::string fetch_data() {
    std::this_thread::sleep_for(std::chrono::seconds(1)); // 模拟耗时操作
    return "从网络获取的数据！";
}

int main() {
    std::future<std::string> future_data = std::async(std::launch::async, fetch_data);

    // // 转移共享状态，原来的 future 被清空  valid() == false
    std::shared_future<std::string> shared_future_data = future_data.share();

    // 多个线程持有一个 shared_future 对象并操作

    // 第一个线程等待结果并访问数据
    std::thread thread1([shared_future_data] {
        std::cout << "线程1：等待数据中..." << std::endl;
        shared_future_data.wait(); // 等待结果可用
        std::cout << "线程1：收到数据：" << shared_future_data.get() << std::endl;
    });

    // 第二个线程等待结果并访问数据
    std::thread thread2([shared_future_data] {
        std::cout << "线程2：等待数据中..." << std::endl;
        shared_future_data.wait();
        std::cout << "线程2：收到数据：" << shared_future_data.get() << std::endl;
    });

    thread1.join();
    thread2.join();

    std::promise<std::string> p;
    std::shared_future<std::string> sf{ p.get_future() }; // 隐式转移所有权
}