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

在单线程的代码中，对象销毁之后再去访问，会产生[未定义行为](https://zh.cppreference.com/w/cpp/language/ub)，线程的生存期增加了这个问题发生的几率。

比如函数结束，那么函数局部对象的生存期都已经结束了，都被销毁了，此时线程函数还持有函数局部对象的指针或引用。

```cpp

```
