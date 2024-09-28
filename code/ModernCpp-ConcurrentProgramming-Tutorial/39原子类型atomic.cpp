#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
using namespace std::chrono_literals;

// 不要这样使用 不要在多线程并发中使用 volatile
// 它的行为是不保证的
std::atomic<int> n = 0;

void read(){
    while(true){
        std::this_thread::sleep_for(500ms);
        std::cout << n.load() << '\n';
    }
}

void write(){
    while (true){
        ++n;
    }
}

// 数据竞争 数据竞争未定义行为
// 优化会假设你的程序中没有未定义行为

// C 语言的平凡的结构体
struct trivial_type {
    int x;
    float y;
};

int main(){
    // 创建一个 std::atomic<trivial_type> 对象
    std::atomic<trivial_type> atomic_my_type{ { 10, 20.5f } };

    // 使用 store 和 load 操作来设置和获取值
    trivial_type new_value{ 30, 40.5f };
    atomic_my_type.store(new_value);

    std::cout << "x: " << atomic_my_type.load().x << ", y: " << atomic_my_type.load().y << std::endl;

    // 使用 exchange 操作
    trivial_type exchanged_value = atomic_my_type.exchange({ 50, 60.5f });
    std::cout << "交换前的 x: " << exchanged_value.x
        << ", 交换前的 y: " << exchanged_value.y << std::endl;
    std::cout << "交换后的 x: " << atomic_my_type.load().x
        << ", 交换后的 y: " << atomic_my_type.load().y << std::endl;
}