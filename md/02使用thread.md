# 使用 `std::thread`

## Hello Word

在我们初学 C++ 的时候，各位应该都写过这样一段代码：

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

其实到目前为止的内容很简单，std::thread 高度封装，其成员函数也很少，我们可以轻易的创建线程执行任务，不过，它的用法也还远不止如此，我们慢慢介绍。
