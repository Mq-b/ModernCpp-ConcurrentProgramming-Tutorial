#include <iostream>
#include <thread>

int global_counter = 0;
__declspec(thread) int thread_local_counter = 0;

void print_counters() {
    std::cout << "global：" << global_counter++ << '\n';
    std::cout << "thread_local：" << thread_local_counter++ << '\n';
}

int main() {
    std::thread{ print_counters }.join();
    std::thread{ print_counters }.join();
}