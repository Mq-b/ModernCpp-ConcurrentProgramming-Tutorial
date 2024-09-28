#include <iostream>
#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <Windows.h>
#pragma comment(lib,"winmm.lib")

using namespace std::chrono_literals;

std::condition_variable cv;
bool done{};
std::mutex m;

bool wait_loop() {
    const auto timeout = std::chrono::steady_clock::now() + 500ms;
    std::unique_lock<std::mutex> lk{ m };
    if (!cv.wait_until(lk, timeout, [] {return done; })) {
        std::cout << "超时 500ms\n";
        return false;
    }
    return true;
}

int main() {
    std::thread t{ wait_loop };
    std::this_thread::sleep_for(400ms);
    done = true;
    cv.notify_one();
    t.join();
}
