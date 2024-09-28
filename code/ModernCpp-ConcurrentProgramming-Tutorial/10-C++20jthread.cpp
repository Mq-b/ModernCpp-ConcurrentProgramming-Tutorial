#include <iostream>
#include <thread>

using namespace std::literals::chrono_literals;

void f(std::stop_token stop_token, int value) {
    while (!stop_token.stop_requested()) { // 检查是否已经收到停止请求
        std::cout << value++ << ' ' << std::flush;
        std::this_thread::sleep_for(200ms);
    }
    std::cout << std::endl;
}

int main() {
    std::jthread thread{ f, 1 }; // 打印 1..15 大约 3 秒
    std::this_thread::sleep_for(3s);
    thread.request_stop(); // 发送信息，线程终止
    std::cout << "乐\n";
    // jthread 的析构函数调用 request_stop() 和 join()。
}