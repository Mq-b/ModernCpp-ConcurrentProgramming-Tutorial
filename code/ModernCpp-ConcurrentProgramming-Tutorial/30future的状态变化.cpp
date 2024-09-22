#include <iostream>
#include <thread>
#include <future>

int main(){
    std::future<void>future = std::async([] {});
    std::cout << std::boolalpha << future.valid() << '\n'; // true
    future.get();
    std::cout << std::boolalpha << future.valid() << '\n'; // false
    try {
        future.get(); // 抛出 future_errc::no_state 异常
    }
    catch (std::exception& e) {
        std::cerr << e.what() << '\n';
    }
}