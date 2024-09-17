#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <map>
#include <string>

class Settings {
private:
    std::map<std::string, std::string> data_;
    mutable std::shared_timed_mutex mutex_; // “M&M 规则”：mutable 与 mutex 一起出现

public:
    void set(const std::string& key, const std::string& value) {
        std::lock_guard<std::shared_timed_mutex> lock{ mutex_ };
        data_[key] = value;
    }

    std::string get(const std::string& key) const {
        std::shared_lock<std::shared_timed_mutex> lock(mutex_);
        auto it = data_.find(key);
        return (it != data_.end()) ? it->second : ""; // 如果没有找到键返回空字符串
    }
};

Settings set;

void read(){
    (void)set.get("1");
}

void write(){
    set.set("1", "a");
}

int main(){
    std::thread t{ read };
    std::thread t2{ read };
    std::thread t3{ read };
    std::thread t4{ read };
    std::thread t5{ write };
    t.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
}