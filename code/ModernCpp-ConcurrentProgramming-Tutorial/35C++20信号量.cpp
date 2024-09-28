#include <iostream>
#include <thread>
#include <chrono>
#include <random>
#include <semaphore>
using namespace std::chrono_literals;

// 定义一个信号量，最大并发数为 3
std::counting_semaphore<3> semaphore{ 3 };

void handle_request(int request_id) {
    // 请求到达，尝试获取信号量
    std::cout << "进入 handle_request 尝试获取信号量\n";

    semaphore.acquire();

    std::cout << "成功获取信号量\n";

    // 此处延时三秒可以方便测试，会看到先输出 3 个“成功获取信号量”，因为只有三个线程能成功调用 acquire，剩余的会被阻塞
    std::this_thread::sleep_for(3s);

    // 模拟处理时间
    std::random_device rd;
    std::mt19937 gen{ rd() };
    std::uniform_int_distribution<> dis(1, 5);
    int processing_time = dis(gen);
    std::this_thread::sleep_for(std::chrono::seconds(processing_time));

    std::cout << std::format("请求 {} 已被处理\n", request_id);

    semaphore.release();
}

int main() {
    // 模拟 10 个并发请求
    std::vector<std::jthread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(handle_request, i);
    }
}