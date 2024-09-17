#include <iostream>
#include <thread>
#include <vector>

std::vector<int> v;

int n = 1;

int main() {
    int cnt = 0;
    auto f = [&] { cnt++; };
    std::thread t1{ f }, t2{ f }, t3{ f }; // ub 未定义行为
    t1.join();
    t2.join();
    t3.join();
    std::cout << cnt << '\n';
}
// 数据竞争它是未定义行为，但是 C++ 的编译器，它会假设你的程序(假设程序是对的，代码是对的)没有任何的未定义行为再去进行优化
// 输出 n，优化，直接缓存这个值