#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <memory>

struct some{
    void do_something(){}
};

std::shared_ptr<some> ptr;
std::once_flag resource_flag;

void init_resource() {
    ptr.reset(new some);
}

void foo() {
    std::call_once(resource_flag, []{ptr.reset(new some); }); // 线程安全的一次初始化
    ptr->do_something();
}

void test(){
    std::call_once(resource_flag, [] {std::cout << "f init\n"; });
}

std::once_flag flag;
int n = 0;

void f() {
    std::call_once(flag, [] {
        ++n;
        std::cout << "第 " << n << " 次调用\n";
        throw std::runtime_error("异常");
    });
}

class my_class{};

inline my_class& get_my_class_instance() {
    static my_class instance;  // 线程安全的初始化过程 初始化严格发生一次
    return instance;
}

int main() {
    get_my_class_instance();
    get_my_class_instance();
    get_my_class_instance();
    get_my_class_instance();
    get_my_class_instance();
}