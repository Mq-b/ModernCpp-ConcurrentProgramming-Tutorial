#include <iostream>
#include <thread>

// https://github.com/Mq-b/Loser-HomeWork/discussions/206

// 反直觉
// 形参、实参
// 函数调用传参，实际上是初始化了（构造）形参的对象

void f(std::thread t) {
    t.join();
}

int main() {
    std::thread t{ [] {} };
    f(std::move(t)); 
    f(std::thread{ [] {} }); 
}