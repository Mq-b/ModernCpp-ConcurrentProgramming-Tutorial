#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <algorithm>
#include <list>
#include <numeric>

std::mutex m;

// 写
void add_to_list(int n, std::list<int>& list) {
    std::vector<int> numbers(n + 1);
    std::iota(numbers.begin(), numbers.end(), 0);
    int sum = std::accumulate(numbers.begin(), numbers.end(), 0);

    {
        std::scoped_lock lc{ m };
        list.push_back(sum);
    }
}

// 读
void print_list(const std::list<int>& list) {
    std::scoped_lock lc{ m };
    for (const auto& i : list) {
        std::cout << i << ' ';
    }
    std::cout << '\n';
}

int main(){
    std::list<int> list;
    std::thread t1{ add_to_list,10,std::ref(list) };
    std::thread t2{ add_to_list,10,std::ref(list) };
    std::thread t3{ print_list,std::cref(list) };
    std::thread t4{ print_list,std::cref(list) };
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    std::cout << "---------------------\n";
    print_list(list);
}