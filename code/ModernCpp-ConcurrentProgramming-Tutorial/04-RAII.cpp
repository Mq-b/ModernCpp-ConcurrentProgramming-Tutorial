#include <iostream>
#include <thread>
#include <exception>

struct func {
    int& m_i;
    func(int& i) :m_i{ i } {}
    void operator()(int n)const {
        for (int i = 0; i <= n; ++i) {
            m_i += i;           // 可能悬空引用
        }
    }
};

void f2(){
    // todo..
    throw std::runtime_error("f2 error");
}

class thread_guard{
public:
    explicit thread_guard(std::thread& t) :thread_{ t }
    {}
    ~thread_guard(){
        std::puts("析构");
        if(thread_.joinable()){ // 如果当前有活跃线程 则进行 join
            thread_.join();
        }
    }
    thread_guard& operator=(const thread_guard&) = delete;
    thread_guard(const thread_guard&) = delete;

    std::thread& thread_;
};

void f() {
    int n = 0;
    std::thread t{ func{n},10 };
    thread_guard g(t);
    f2(); // 可能抛出异常
}

int main(){
    // 栈回溯 
    try{
        f();
    }catch (...){}
}