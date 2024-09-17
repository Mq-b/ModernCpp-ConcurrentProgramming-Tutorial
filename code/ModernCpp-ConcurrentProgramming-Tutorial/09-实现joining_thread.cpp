#include <iostream>
#include <thread>
#include <type_traits>
#include <vector>
#include <mutex>

class joining_thread {
    std::thread t;
public:
    joining_thread()noexcept = default;

    template<typename Callable, typename... Args>
    explicit joining_thread(Callable&& func, Args&&...args) :
        t{ std::forward<Callable>(func), std::forward<Args>(args)... } {}

    explicit joining_thread(std::thread t_)noexcept : t{ std::move(t_) } {}

    joining_thread(joining_thread&& other)noexcept : t{ std::move(other.t) } {}

    joining_thread& operator=(std::thread&& other)noexcept {
        if (joinable()) { // 如果当前有活跃线程（判断当前对象是否持有资源），那就先执行完（先释放）
            join(); // 就相当于释放资源一样的意思
        }
        t = std::move(other);
        return *this;
    }
    ~joining_thread() {
        if (joinable()) {
            join();
        }
    }
    void swap(joining_thread& other)noexcept {
        t.swap(other.t);
    }
    std::thread::id get_id()const noexcept {
        return t.get_id();
    }
    bool joinable()const noexcept {
        return t.joinable();
    }
    void join() {
        t.join();
    }
    void detach() {
        t.detach();
    }
    std::thread& data()noexcept {
        return t;
    }
    const std::thread& data()const noexcept {
        return t;
    }
};

int main(){
    auto func = []{
        std::cout << std::this_thread::get_id() << '\n';
    };

    std::vector<std::thread> vec;

    for (int i = 0; i < 10; ++i){
        vec.emplace_back(func);
    }

    for(auto& thread : vec){
        thread.join();
    }
}