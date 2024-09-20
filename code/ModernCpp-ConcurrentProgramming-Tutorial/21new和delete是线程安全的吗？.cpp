#include <iostream>
#include <thread>
#include <mutex>

struct X {
    X(int v) { // 主要防止有人认为构造函数、析构函数啊，是线程安全的
        std::cout << v << " 🤣\n";
    }
};

void f() {
    X* p = new X{ 1 }; // 存在数据竞争
    delete p;
}

int main()
{
    for (int i = 0; i < 10; ++i) {
        std::thread t{ f };
        std::thread t2{ f };
        t.join();
        t2.join();
    }

    // C++ 保证的是内存的申请和释放 这种全局状态 是线程安全的
}