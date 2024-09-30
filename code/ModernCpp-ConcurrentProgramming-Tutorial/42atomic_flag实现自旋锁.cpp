#include <iostream>
#include <atomic>
#include <mutex>
#include <thread>

class spinlock_mutex {
    std::atomic_flag flag{};
public:
    spinlock_mutex()noexcept = default;
    void lock()noexcept {
        while (flag.test_and_set(std::memory_order_acquire));
    }

    void unlock()noexcept {
        flag.clear(std::memory_order_release);
    }
};

spinlock_mutex m;

void f() {
    std::lock_guard<spinlock_mutex> lc{ m };
    std::cout << "😅😅" << "❤️❤️\n";
}

int main(){
    std::thread t{ f };
    std::thread t1{ f };
    std::thread t2{ f };
    std::thread t3{ f };
    std::thread t4{ f };
    std::thread t5{ f };
    t.join();
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
}