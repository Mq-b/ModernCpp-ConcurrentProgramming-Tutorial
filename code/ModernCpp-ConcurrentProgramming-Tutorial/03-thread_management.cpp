#include <iostream>
#include <thread>

struct func {
    int& m_i;
    func(int& i) :m_i{ i } {}
    void operator()(int n)const {
        for (int i = 0; i <= n; ++i) {
            m_i += i;           // 可能悬空引用
        }
    }
};

void f2() { throw std::runtime_error("test f2()"); }

void f() {
    int n = 0;
    std::thread t{ func{n},10 };
    try {
        // todo.. 一些当前线程可能抛出异常的代码
        f2();
        t.join();
    }
    catch (...) {
        t.join(); // 1
        // 如果此处不抛出 会掩盖错误 我们根本没有处理 没有解决
    }
}

int main() {
    f();
}