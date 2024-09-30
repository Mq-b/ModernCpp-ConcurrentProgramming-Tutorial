#include <iostream>
#include <memory>
#include <thread>
#include <chrono>
#include <syncstream>
using namespace std::chrono_literals;

class Data {
public:
    Data(int value = 0) : value_(value) {}
    int get_value() const { return value_; }
    void set_value(int new_value) { value_ = new_value; }
private:
    int value_;
};

std::atomic<std::shared_ptr<Data>> data = std::make_shared<Data>();

void writer() {
    for (int i = 0; i < 10; ++i) {
        std::shared_ptr<Data> new_data = std::make_shared<Data>(i);
        data.store(new_data); 
        std::this_thread::sleep_for(100ms);
    }
}

void reader() {
    for (int i = 0; i < 10; ++i) {
        if (auto sp = data.load()) {
            std::cout << "读取线程值: " << sp->get_value() << std::endl;
        }
        else {
            std::cout << "没有读取到数据" << std::endl;
        }
        std::this_thread::sleep_for(100ms);
    }
}

std::atomic<std::shared_ptr<int>> ptr = std::make_shared<int>();

void wait_for_wake_up() {
    std::osyncstream{ std::cout }
        << "线程 "
        << std::this_thread::get_id()
        << " 阻塞，等待更新唤醒\n";

    // 等待 ptr 变为其它值
    ptr.wait(ptr.load());

    std::osyncstream{ std::cout }
        << "线程 "
        << std::this_thread::get_id()
        << " 已被唤醒\n";
}

void wake_up() {
    std::this_thread::sleep_for(5s);

    // 更新值并唤醒
    ptr.store(std::make_shared<int>(10));
    ptr.notify_one();
}

int main() {
    //std::thread writer_thread{ writer };
    //std::thread reader_thread{ reader };

    //writer_thread.join();
    //reader_thread.join();

    //std::atomic<std::shared_ptr<int>> ptr = std::make_shared<int>(10);
    //std::atomic_ref<int> ref{ *ptr.load() };
    //ref = 100; // 原子地赋 100 给被引用的对象
    //std::cout << *ptr.load() << '\n';
    std::thread t1{ wait_for_wake_up };
    wake_up();
    t1.join();
}