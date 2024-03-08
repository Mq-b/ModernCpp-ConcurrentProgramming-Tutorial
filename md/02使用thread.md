# 使用 `std::thread`

## Hello Word

在我们初学 C++ 的时候应该都写过这样一段代码：

```cpp
#include <iostream>

int main(){
    std::cout << "Hello Word!" << std::endl;
}
```

这段代码将"Hello Word!"写入到标准输出流，换行并[刷新](https://zh.cppreference.com/w/cpp/io/manip/endl)。

我们启动一个线程来做这件事情：

```cpp
#include <iostream>
#include <thread>  // 引入线程支持头文件

void hello(){     // 定义一个函数用作打印任务
    std::cout << "Hello Word" << std::endl;
}

int main(){
    std::thread t{ hello };
    t.join();
}
```

`std::thread t{ hello };` 创建了一个线程对象 `t`，将 `hello` 作为它的[可调用(Callable)](https://zh.cppreference.com/w/cpp/named_req/Callable)对象，在新线程中执行。线程对象关联了一个线程资源，我们无需手动控制，在线程对象构造成功之后，就自动在新线程开始执行函数 `hello`。

`t.join();` 等待线程对象 `t` 关联的线程执行完毕，否则将一直堵塞。这里是必须调用的，否则 `std::thread` 的析构函数将调用 `std::terminate()`。

这是因为我们创建线程对象 `t` 的时候就关联了一个活跃的线程，调用 `join()` 就是确保线程对象关联的线程已经执行完毕，然后会修改对象的状态，让 [`std::thread::joinable()`](https://zh.cppreference.com/w/cpp/thread/thread/joinable) 返回 `false`，表示线程对象目前没有关联活跃线程。`std::thread` 的析构函数，正是通过 `joinable()` 判断线程对象目前是否有关联活跃线程，如果为 `true`，那么就当做有关联活跃线程，会调用 `std::terminate()`。

---

如你所见，std::thread 高度封装，其成员函数也很少，我们可以轻易的创建线程执行任务，不过，它的用法也还远不止如此，我们慢慢介绍。

## 线程管理

在 C++ 标准库 中，只能管理与 `std::thread` 关联的线程，类 `std::thread` 的对象就是指代线程的对象，我们说“线程管理”，其实也就是管理 `std::thread` 对象。

### 启动新线程

使用 C++ 线程库启动线程，就是构造 std::thread 对象。

> 当然了，如果是**默认构造**，那么 `std::thread` 线程对象没有关联线程的，自然也不会启动线程执行任务。
>
> ```cpp
> std::thread t; //  构造不表示线程的新 std::thread 对象-
> ```

我们上一节的示例是传递了一个函数给 `std::thread` 对象，函数会在新线程中执行。`std::thread` 支持的形式还有很多，只要是[可调用(Callable)](https://zh.cppreference.com/w/cpp/named_req/Callable)对象即可，比如重载了 `operator()` 的类对象（也可以直接叫函数对象）。

```cpp
class Task{
public:
    void operator()()const {
        std::cout << "operator()()const\n";
    }
};
```

我们显然没办法直接像函数使用函数名一样，使用“类名”，函数名可以隐式转换到指向它的函数指针，我们要用使用 `Task` 就得创造对象了

```cpp
std::thread t{ Task{} };
t.join();
```

直接创建临时对象即可，因为创建局部的也没什么别的作用。

不过有件事情需要注意，当我们使用函数对象用于构造 `std::thread` 的时候，如果你传入的是一个临时对象，且使用的**都是 “`()`”小括号初始化**，那么**编译器会将此语法解析为函数声明**。

```cpp
std::thread t( Task() ); // 函数声明
```

这被编译器解析为函数声明，是一个返回类型为 `std::thread`，函数名为 `t`，接受一个返回 `Task` 的空参的函数指针类型，也就是 `Task(*)()`。

之所以我们看着抽象是因为这里的形参是无名的，且写了个函数类型。

我们用一个简单的示例为你展示：

```cpp
void h(int(int));         //#1 声明
void h(int (*p)(int)){}   //#2 定义
```

即使我还没有为你讲述概念，我相信你也发现了，#1 和 #2 的区别无非是，#1 省略了形参的名称，还有它的形参是函数类型而不是函数指针类型，没有 **`*`**。

> 在确定每个形参的类型后，类型是 “T 的数组”或某个**函数类型 T 的形参会调整为具有类型“指向 T 的指针”**。[文档](https://zh.cppreference.com/w/cpp/language/function#.E5.BD.A2.E5.8F.82.E7.B1.BB.E5.9E.8B.E5.88.97.E8.A1.A8)。

显然，`int(int)` 是一个函数类型，它被调整为了一个指向这个函数类型的指针类型。

那么回到我们最初的：

```cpp
std::thread t( Task() );                    // #1 函数声明
std::thread t( Task (*p)() ){ return {}; }  // #2 函数定义
```

`#2`我们写出了函数形参名称 `p`，再将函数类型写成函数指针类型，事实上**完全等价**。我相信，这样，也就足够了。

所以总而言之，建议使用 `{}` 进行初始化，这是好习惯，大多数时候它是合适的。

C++11 引入的 Lambda 表达式，同样可以作为构造 `std::thread` 的参数，因为 Lambda 本身就是[生成](https://cppinsights.io/s/c448ad3d)了一个函数对象。

```cpp
#include <iostream>
#include <thread>

int main(){
    std::thread thread{ [] {std::cout << "Hello Word!\n"; } };
    thread.join();
}
```

---

启动线程后（也就是构造 `std::thread` 对象）我们必须在线程对象的生存期结束之前，即 [`std::thread::~thread`](https://zh.cppreference.com/w/cpp/thread/thread/%7Ethread) 调用之前，决定它的执行策略，是 [`join()`](https://zh.cppreference.com/w/cpp/thread/thread/join)（合并）还是 [`detach()`](https://zh.cppreference.com/w/cpp/thread/thread/detach)（分离）。

我们先前使用的就是 join()，我们聊一下 **detach()**，当 `std::thread` 线程对象调用了 detach()，那么就是线程对象放弃了对线程资源的所有权，不再管理此线程，允许此线程独立的运行，在线程退出时释放所有分配的资源。

放弃了对线程资源的所有权，也就是线程对象没有关联活跃线程了，此时 joinable 为 **`false`**。

在单线程的代码中，对象销毁之后再去访问，会产生[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)，多线程增加了这个问题发生的几率。

比如函数结束，那么函数局部对象的生存期都已经结束了，都被销毁了，此时线程函数还持有函数局部对象的指针或引用。

```cpp
#include <iostream>
#include <thread>

struct func{
    int& m_i;
    func(int& i) :m_i{ i } {}
    void operator()(int n){
        for(int i=0;i<=n;++i){
            m_i += i;           // 可能悬空引用
        }
    }
};

int main(){
    int n = 0;
    std::thread my_thread{ func{n},100 };
    my_thread.detach();        // 分离，不等待线程结束
}                              // 分离的线程可能还在运行
```

1. 主线程（main）创建局部对象 n、创建线程对象 my_thread 启动线程，执行任务 **`func{n}`**，局部对象 n 的引用被子线程持有。传入 100 用于调用 func 的 operator(int)。

2. `my_thread.detach();`，joinable() 为 `false`。线程分离，线程对象不再持有线程资源，线程独立的运行。

3. 主线程不等待，此时分离的子线程可能没有执行完毕，但是主线程（main）已经结束，局部对象 `n` 生存期结束，被销毁，而此时子线程还持有它的引用，访问悬空引用，造成未定义行为。`my_thread` 已经没有关联线程资源，正常析构，没有问题。

解决方法很简单，将 detach() 替换为 join()。

>**通常非常不推荐使用 detach()，因为程序员必须确保所有创建的线程正常退出，释放所有获取的资源并执行其他必要的清理操作。这意味着通过调用 detach() 放弃线程的所有权不是一种选择，因此 join 应该在所有场景中使用。** 一些老式特殊情况不聊。

另外提示一下，也**不要想着** detach() 之后，再次调用 join()

```cpp
my_thread.detach();
// todo..
my_thread.join();
// 函数结束
```

认为这样可以确保被分离的线程在这里阻塞执行完？

我们前面聊的很清楚了，detach() 是线程分离，**线程对象放弃了线程资源的所有权**，此时我们的 my_thread 它现在根本没有关联任何线程。调用 join() 是：“阻塞当前线程直至 *this 所标识的线程结束其执行”，我们的**线程对象都没有线程，堵塞什么？执行什么呢？**

简单点说，必须是 std::thread 的 joinable() 为 true 即线程对象有活跃线程，才能调用 join() 和 detach()。

顺带的，我们还得处理线程运行后的异常问题，举个例子：你在一个函数中构造了一个 std::thread 对象，线程开始执行，函数继续执行下面别的代码，但是如果抛出了异常呢？下面我的 **join() 就会被跳过**。

```cpp
std::thread my_thread{func{n},10};
//todo.. 抛出异常的代码
my_thread.join();
```

避免程序被抛出的异常所终止，在异常处理过程中调用 join()，从而避免线程对象析构产生问题。

```cpp
struct func; // 复用之前
void f(){
    int n = 0;
    std::thread t{ func{n},10 };
    try{
        // todo.. 一些当前线程可能抛出异常的代码
        f2();
    }
    catch (...){
        t.join(); // 1
        throw;
    }
    t.join();    // 2
}
```

我知道你可能有很多疑问，我们既然 catch 接住了异常，为什么还要 throw？以及为什么我们要两个 join()？

这两个问题其实也算一个问题，如果代码里抛出了异常，就会跳转到 catch 的代码中，执行 join() 确保线程正常执行完成，线程对象可以正常析构。然而此时我们必须再次 throw 抛出异常，因为你要是不抛出，那么你不是还得执行一个 `t.join()`？显然逻辑不对，自然抛出。至于这个**函数产生的异常，由调用方进行处理**，我们只是确保函数 f 中创建的线程正常执行完成，其局部对象正常析构释放。[测试代码](https://godbolt.org/z/33ajh893P)。

### RAII

“[资源获取即初始化](https://zh.cppreference.com/w/cpp/language/raii)”(RAII，Resource Acquisition Is Initialization)。

简单的理解是：构造函数申请资源，析构函数释放资源，让对象的生命周期和资源绑定。

我们可以提供一个类，在析构函数中使用 join() 确保线程执行完成，线程对象正常析构。

```cpp

```
