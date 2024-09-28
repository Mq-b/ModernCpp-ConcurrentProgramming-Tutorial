#include <iostream>
#include <thread>
#include <future>
#include <chrono>
using namespace std::chrono_literals;

void f(std::promise<int> obj ,int num){
    // todo..
    obj.set_value(num * num); // 调用了 set_value
    // todo..
    std::this_thread::sleep_for(5s); // 模拟一些计算
}

void throw_function(std::promise<int> prom) {
    prom.set_value(100);
    try {
        // todo..
        throw std::runtime_error("一个异常");
    }
    catch (...) {
        try {
            // 共享状态的 promise 已存储值，调用 set_exception 产生异常
            prom.set_exception(std::current_exception());
        }
        catch (std::exception& e) {
            std::cerr << "来自 set_exception 的异常: " << e.what() << '\n';
        }
    }
}

int main() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    std::thread t(throw_function, std::move(prom));

    std::cout << "等待线程执行，抛出异常并设置\n";
    std::cout << "值：" << fut.get() << '\n'; // 100

    t.join();
}


//int main(){
//    std::promise<int> promise;
//
//    auto future = promise.get_future(); // 关联了
//
//    std::thread t{ f,std::move(promise), 10 };
//    // f(std::move(promise), 10);
//
//    std::cout << future.get() << '\n'; // 阻塞，直至结果可用
//    std::cout << "end\n";
//    t.join();
//}