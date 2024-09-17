#include <iostream>
#include <thread>
#include <mutex>

class Data {
    int a{};
    std::string b{};
public:
    void do_something() {
        // 修改数据成员等...
    }
};

class Data_wrapper {
    Data data;
    std::mutex m;
public:
    template<class Func>
    void process_data(Func func) {
        std::lock_guard<std::mutex> lc{ m };
        func(data);  // 受保护数据传递给函数
    }
};

Data* p = nullptr;

void malicious_function(Data& protected_data) {
    p = &protected_data; // 受保护的数据被传递到外部
}

Data_wrapper d;

void foo() {
    d.process_data(malicious_function);  // 传递了一个恶意的函数
    p->do_something();                   // 在无保护的情况下访问保护数据
}

int main(){
    
}