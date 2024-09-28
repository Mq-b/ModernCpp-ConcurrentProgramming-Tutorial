#include <iostream>
#include <thread>

thread_local int n = (std::puts("thread_local init"), 0);

void f(){
    (void)n; // 防止 gcc 与 clang 优化
    std::puts("f");
}

void f2(){
    thread_local static int n = (std::puts("f2 init"), 0);
}

int main(){
    (void)n; // 防止 gcc 与 clang 优化
    std::cout << "main\n";
    std::thread{ f }.join();
    f2();
    f2();
    f2();
}

// gcc 与 clang 存在优化，会出现与 msvc 不同的结果，它们直接将线程变量优化掉了
// 这应该视作 bug。
// 视频中想到 std::async 是下一章的内容跳过了（想到的是 msvc 的一个问题），忘记了 gcc 与 clang 此处也存在问题。
// https://godbolt.org/z/qa6YfMqP7