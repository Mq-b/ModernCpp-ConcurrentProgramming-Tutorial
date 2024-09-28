#include <iostream>
#include <omp.h>
#include <string>
#include <thread>

void f(int start, int end, int thread_id) {
    for (int i = start; i <= end; ++i) {
        // 输出当前线程的数字
        std::cout << std::to_string(i) + " ";

        // 等待所有线程同步到达 barrier 也就是等待都输出完数字
#pragma omp barrier

// 每个线程输出完一句后，主线程输出轮次信息
#pragma omp master
        {
            static int round_number = 1;
            std::cout << "\t第" << round_number++ << "轮结束\n";
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        // 再次同步 等待所有线程（包括主线程）到达此处、避免其它线程继续执行打断主线程的输出
#pragma omp barrier
    }
}

int main() {
    constexpr int num_threads = 10;
    omp_set_num_threads(num_threads);

#pragma omp parallel
    {
        const int thread_id = omp_get_thread_num();
        f(thread_id * 10 + 1, (thread_id + 1) * 10, thread_id);
    }

}

// https://godbolt.org/z/fabqhbx3P