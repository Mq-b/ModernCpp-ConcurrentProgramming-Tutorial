#include <iostream>
#include <thread>
#include <mutex>

std::recursive_mutex mtx;

void recursive_function(int count) {
    std::lock_guard<std::recursive_mutex> lc{ mtx };
    std::cout << "Locked by thread: " << std::this_thread::get_id() << ", count: " << count << std::endl;
    if (count > 0) {
        recursive_function(count - 1);
    }
}

int main() {
    std::thread t1(recursive_function, 3);
    std::thread t2(recursive_function, 2);

    t1.join();
    t2.join();
}