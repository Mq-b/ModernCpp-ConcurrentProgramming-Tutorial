#include <iostream>
#include <thread>
#include <vector>

struct X {
    X() {
        // 假设 X 的初始化没那么快
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::puts("X");
        v.resize(10, 6);
    }
    std::vector<int> v;
};

struct Test {
    Test()/* : t{ &Test::f, this }*/ // 线程已经开始执行
    {
         // 严格意义来说 这里不算初始化 至少不算 C++ 标准的定义
    }
    void start()
    {
        t = std::thread{ &Test::f, this };
    }
    ~Test() {
        if (t.joinable())
            t.join();
    }
    void f()const { // 如果在函数执行的线程 f 中使用 x 则会存在问题。使用了未初始化的数据成员 ub
        std::cout << "f\n";
        std::cout << x.v[9] << '\n';
    }

    
    std::thread t; // 声明顺序决定了初始化顺序，优先初始化 t
    X x;
};

int main() {
    Test t;
    t.start();
}