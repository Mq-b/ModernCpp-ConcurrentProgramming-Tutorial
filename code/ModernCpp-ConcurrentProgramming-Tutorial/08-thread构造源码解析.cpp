#include <iostream>
#include <thread>
#include <type_traits>

struct X {
    X(X&& x)noexcept {}
    template <class Fn, class... Args, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<Fn>, X>, int> = 0>
    X(Fn&& f, Args&&...args) {}
    X(const X&) = delete;
};

int main(){
    std::thread
    X x{ [] {} };
    X x2{ x }; // 选择到了有参构造函数，不导致编译错误
}