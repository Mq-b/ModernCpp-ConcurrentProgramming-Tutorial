#include <iostream>
#include <thread>
#include <functional>

void f(const std::string&);

void test() {
    char buffer[1024]{};
    //todo.. code
    std::thread t{ f, std::string(buffer) }; // std::string(buffer) 构造对象，由 std::string 对象自行管理
    t.detach();
}

int main(){
    // A 的引用只能引用 A 类型，或者以任何形式 转换到 A
    double a = 1;
    const int& p = a; // a 隐式转换到了 int 类型，这个转换是纯右值表达式
    // 因为 const T& 可以接右值表达式，所以才能通过编译
    const std::string& s = "123"; // "123" 构造了 std::string 对象
}