#include <iostream>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <queue>
#include <functional>
#include <thread>
#include <vector>
#include <future>
#include <memory>
#include <syncstream>
using namespace std::chrono_literals;

inline std::size_t default_thread_pool_size() noexcept{
    std::size_t num_threads = std::thread::hardware_concurrency();
    num_threads = num_threads == 0 ? 2 : num_threads; // 防止无法检测当前硬件，让我们线程池至少有 2 个线程
    return num_threads;
}

class ThreadPool{
public:
    using Task = std::packaged_task<void()>;

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    ThreadPool(std::size_t num_thread = default_thread_pool_size()) :
        stop_{ false }, num_thread_{ num_thread }
    {
        start();
    }
    ~ThreadPool(){
        stop();
    }

    void stop(){
        stop_ = true;
        cv_.notify_all();
        for (auto& thread : pool_){
            if (thread.joinable())
                thread.join();
        }
        pool_.clear();
    }

    template<typename F, typename ...Args>
    std::future<std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>> submit(F&& f, Args&&...args){
        using RetType = std::invoke_result_t<std::decay_t<F>, std::decay_t<Args>...>;
        if(stop_){
            throw std::runtime_error("ThreadPool is stopped");
        }
        auto task = std::make_shared<std::packaged_task<RetType()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));
        
        std::future<RetType> ret = task->get_future();

        {
            std::lock_guard<std::mutex> lc{ mutex_ };
            tasks_.emplace([task] {(*task)(); });
        }
        cv_.notify_one();

        return ret;
    }

    void start(){
        for (std::size_t i = 0; i < num_thread_; ++i){
            pool_.emplace_back([this]{
                while (!stop_) {
                    Task task;
                    {
                        std::unique_lock<std::mutex> lock{ mutex_ };
                        cv_.wait(lock, [this] {return stop_ || !tasks_.empty(); });
                        if (tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

private:
    std::mutex               mutex_;
    std::condition_variable  cv_;
    std::atomic<bool>        stop_;
    std::atomic<std::size_t> num_thread_;
    std::queue<Task>         tasks_;
    std::vector<std::thread> pool_;
};

int print_task(int n) {
    std::osyncstream{ std::cout } << "Task " << n << " is running on thr: " <<
        std::this_thread::get_id() << '\n';
    return n;
}
int print_task2(int n) {
    std::osyncstream{ std::cout } << "🐢🐢🐢 " << n << " 🐉🐉🐉" << std::endl;
    return n;
}

struct X {
    void f(const int& n) const {
        std::osyncstream{ std::cout } << &n << '\n';
    }
};

int main() {
    ThreadPool pool{ 4 }; // 创建一个有 4 个线程的线程池

    X x;
    int n = 6;
    std::cout << &n << '\n';
    auto t = pool.submit(&X::f, &x, n); // 默认复制，地址不同
    auto t2 = pool.submit(&X::f, &x, std::ref(n));
    t.wait();
    t2.wait();
} // 析构自动 stop()自动 stop() 