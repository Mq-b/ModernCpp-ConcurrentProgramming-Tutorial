#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <queue>
using namespace std::chrono_literals;

template<typename T>
class threadsafe_queue {
    mutable std::mutex m;              // M&M 原则 互斥量，用于保护队列操作的独占访问
    std::condition_variable data_cond; // 条件变量，用于在队列为空时等待
    std::queue<T> data_queue;          // 实际存储数据的队列
public:
    threadsafe_queue() {}

    void push(T new_value) {
        {
            std::lock_guard<std::mutex> lk{ m };
            std::cout << "push:" << new_value << std::endl;
            data_queue.push(new_value);
        }
        data_cond.notify_one();
    }
    // 从队列中弹出元素（阻塞直到队列不为空）
    void pop(T& value) {
        std::unique_lock<std::mutex> lk{ m };
        data_cond.wait(lk, [this] {return !data_queue.empty(); }); // 解除阻塞 重新获取锁 lock
        value = data_queue.front();
        std::cout << "pop:" << value << std::endl;
        data_queue.pop();
    }
    // 从队列中弹出元素（阻塞直到队列不为空），并返回一个指向弹出元素的 shared_ptr
    std::shared_ptr<T> pop() {
        std::unique_lock<std::mutex> lk{ m };
        data_cond.wait(lk, [this] {return !data_queue.empty(); });
        std::shared_ptr<T>res{ std::make_shared<T>(data_queue.front()) };
        data_queue.pop();
        return res;
    }
    bool empty()const {
        std::lock_guard<std::mutex> lk(m);
        return data_queue.empty();
    }
};

void producer(threadsafe_queue<int>& q) {
    for (int i = 0; i < 5; ++i) {
        q.push(i);
    }
}
void consumer(threadsafe_queue<int>& q) {
    for (int i = 0; i < 5; ++i) {
        int value{};
        q.pop(value);
    }
}

int main() {
    threadsafe_queue<int> q;

    std::thread producer_thread(producer, std::ref(q));
    std::thread consumer_thread(consumer, std::ref(q));

    producer_thread.join();
    consumer_thread.join();
}