#include <iostream>
#include <thread>
#include <future>

template<typename R, typename...Ts, typename...Args>
void async_task(std::packaged_task<R(Ts...)>& task, Args&&...args) {
    // todo..
    task(std::forward<Args>(args)...);
}

int main() {
    std::packaged_task<int(int, int)> task([](int a, int b) {
        return a + b;
    });

    int value = 50;

    std::future<int> future = task.get_future();

    // 创建一个线程来执行异步任务
    std::thread t{ [&] { async_task(task, value, value); } };
    std::cout << future.get() << '\n';
    t.join();
}

//int main(){
//    std::cout << "main: " << std::this_thread::get_id() << '\n';
//
//    // 只能移动不能复制
//    std::packaged_task<double(int, int)> task{ [](int a, int b) {
//        std::cout << "packaged_task: " << std::this_thread::get_id() << '\n';
//        return std::pow(a, b);
//    } };
//
//    std::future<double> future = task.get_future();
//
//    // task(10, 2); // 调用 此处执行任务
//
//    std::thread t{ std::move(task) ,10,2 };
//
//    std::cout << "------\n";
//
//    std::cout << future.get() << '\n'; // 会阻塞，直到任务执行完毕
//
//    t.join();
//}