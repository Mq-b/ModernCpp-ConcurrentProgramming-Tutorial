#include <iostream>

struct X{
    int v{};
    void f()const {
        std::cout << v << '\n';
    }
};

int main(){
    int arr[10]{ 1,2 };

    std::atomic<int*> p{ arr };

    p.fetch_add(1);
    std::cout << *(p.load()) << '\n';

    p.fetch_sub(1);
    std::cout << *(p.load()) << '\n';

    p += 1;
    std::cout << *(p.load()) << '\n';

    p -= 1;
    std::cout << *(p.load()) << '\n';

    X xs[3]{ {10},{20},{30} };
    std::atomic<X*> p2{ xs };
    p2.load()->f();
    p2 += 2;
    p2.load()->f();
}