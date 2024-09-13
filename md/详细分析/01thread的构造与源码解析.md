# `std::thread` 的构造-源码解析

我们这单章是为了专门解释一下 C++11 引入的 `std::thread` 是如何构造的，是如何创建线程传递参数的，让你彻底了解这个类。

我们以 **MSVC** 实现的 [`std::thread`](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/thread) 代码进行讲解，MSVC STL 很早之前就不支持 C++11 了，它的实现完全基于 **C++14**，出于某些原因 **C++17** 的一些库（如 [`invoke`](https://zh.cppreference.com/w/cpp/utility/functional/invoke)， _v 变量模板）被向后移植到了 **C++14** 模式，所以即使是 C++11 标准库设施，实现中可能也是使用到了 C++14、17 的东西。

## `std::thread` 的数据成员

- **了解一个庞大的类，最简单的方式就是先看它的数据成员有什么**。

`std::thread` 只保有一个私有数据成员 [`_Thr`](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/thread#L163)：

```cpp
private:
    _Thrd_t _Thr;
```

[`_Thrd_t`](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/__msvc_threads_core.hpp#L20-L24) 是一个结构体，它保有两个数据成员：

```cpp
using _Thrd_id_t = unsigned int;
struct _Thrd_t { // thread identifier for Win32
    void* _Hnd; // Win32 HANDLE
    _Thrd_id_t _Id;
};
```

结构很明确，这个结构体的 `_Hnd` 成员是指向线程的句柄，`_Id` 成员就是保有线程的 ID。

在64 位操作系统，因为内存对齐，指针 8 ，无符号 int 4，这个结构体 `_Thrd_t` 就是占据 16 个字节。也就是说 `sizeof(std::thread)` 的结果应该为 **16**。

## `std::thread` 的构造函数

`std::thread` 有四个[构造函数](https://zh.cppreference.com/w/cpp/thread/thread/thread)，分别是：

1. 默认构造函数，构造不关联线程的新 std::thread 对象。

   ```cpp
   thread() noexcept : _Thr{} {}
   ```

   [值初始化](https://zh.cppreference.com/w/cpp/language/value_initialization#:~:text=%E5%87%BD%E6%95%B0%E7%9A%84%E7%B1%BB%EF%BC%89%EF%BC%8C-,%E9%82%A3%E4%B9%88%E9%9B%B6%E5%88%9D%E5%A7%8B%E5%8C%96%E5%AF%B9%E8%B1%A1,-%EF%BC%8C%E7%84%B6%E5%90%8E%E5%A6%82%E6%9E%9C%E5%AE%83)了数据成员  _Thr ，这里的效果相当于给其成员 `_Hnd` 和 `_Id` 都进行[零初始化](https://zh.cppreference.com/w/cpp/language/zero_initialization)。

2. 移动构造函数，转移线程的所有权，构造 other 关联的执行线程的 `std::thread` 对象。此调用后 other 不再表示执行线程失去了线程的所有权。

   ```cpp
   thread(thread&& _Other) noexcept : _Thr(_STD exchange(_Other._Thr, {})) {}
   ```

   [_STD](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/yvals_core.h#L1934) 是一个宏，展开就是 `::std::`，也就是 [`::std::exchange`](https://zh.cppreference.com/w/cpp/utility/exchange)，将 `_Other._Thr` 赋为 `{}` （也就是置空），返回 `_Other._Thr` 的旧值用以初始化当前对象的数据成员 `_Thr`。

3. 复制构造函数被定义为弃置的，std::thread 不可复制。两个 std::thread 不可表示一个线程，std::thread 对线程资源是独占所有权。

   ```cpp
   thread(const thread&) = delete;
   ```

4. 构造新的 `std::thread` 对象并将它与执行线程关联。**表示新的执行线程开始执行**。

   ```cpp
   template <class _Fn, class... _Args, enable_if_t<!is_same_v<_Remove_cvref_t<_Fn>, thread>, int> = 0>
       _NODISCARD_CTOR_THREAD explicit thread(_Fn&& _Fx, _Args&&... _Ax) {
           _Start(_STD forward<_Fn>(_Fx), _STD forward<_Args>(_Ax)...);
       }
   ```

---

前三个构造函数都没啥要特别聊的，非常简单，只有第四个构造函数较为复杂，且是我们本章重点，需要详细讲解。（*注意 MSVC 使用标准库的内容很多时候不加 **std::**，脑补一下就行*）

如你所见，这个构造函数本身并没有做什么，它只是一个可变参数成员函数模板，增加了一些 [SFINAE](https://zh.cppreference.com/w/cpp/language/sfinae) 进行约束我们传入的[可调用](https://zh.cppreference.com/w/cpp/named_req/Callable)对象的类型不能是 `std::thread`。关于这个约束你可能有问题，因为 `std::thread` 他并没有 `operator()` 的重载，不是可调用类型，这个 `enable_if_t` 的意义是什么呢？其实很简单，如下：

```cpp
struct X{
    X(X&& x)noexcept{}
    template <class Fn, class... Args>
    X(Fn&& f,Args&&...args){}
    X(const X&) = delete;
};

X x{ [] {} };
X x2{ x }; // 选择到了有参构造函数，不导致编译错误
```

以上这段代码可以正常的[通过编译](https://godbolt.org/z/6zhW6xjqP)。这是重载决议的事情，我们知道，`std::thread` 是不可复制的，这种代码自然不应该让它通过编译，选择到我们的有参构造，所以我们添加一个约束让其不能选择到我们的有参构造：

```cpp
template <class Fn, class... Args, std::enable_if_t<!std::is_same_v<std::remove_cvref_t<Fn>, X>, int> = 0>
```

这样，这段代码就会正常的出现[编译错误](https://godbolt.org/z/Mc1h1GcdT)，信息如下：

```txt
error C2280: “X::X(const X &)”: 尝试引用已删除的函数
note: 参见“X::X”的声明
note: “X::X(const X &)”: 已隐式删除函数
```

也就满足了我们的要求，重载决议选择到了弃置复制构造函数产生编译错误，这也就是源码中添加约束的目的。

而构造函数体中调用了一个函数 [**`_Start`**](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/thread#L72-L87)，将我们构造函数的参数全部完美转发，去调用它，这个函数才是我们的重点，如下：

```cpp
template <class _Fn, class... _Args>
void _Start(_Fn&& _Fx, _Args&&... _Ax) {
    using _Tuple                 = tuple<decay_t<_Fn>, decay_t<_Args>...>;
    auto _Decay_copied           = _STD make_unique<_Tuple>(_STD forward<_Fn>(_Fx), _STD forward<_Args>(_Ax)...);
    constexpr auto _Invoker_proc = _Get_invoke<_Tuple>(make_index_sequence<1 + sizeof...(_Args)>{});

    _Thr._Hnd =
        reinterpret_cast<void*>(_CSTD _beginthreadex(nullptr, 0, _Invoker_proc, _Decay_copied.get(), 0, &_Thr._Id));

    if (_Thr._Hnd) { // ownership transferred to the thread
        (void) _Decay_copied.release();
    } else { // failed to start thread
        _Thr._Id = 0;
        _Throw_Cpp_error(_RESOURCE_UNAVAILABLE_TRY_AGAIN);
    }
}
```

1. 它也是一个可变参数成员函数模板，接受一个可调用对象 `_Fn` 和一系列参数 `_Args...` ，这些东西用来创建一个线程。

2. `using _Tuple = tuple<decay_t<_Fn>, decay_t<_Args>...>`

   - 定义了一个[元组](https://zh.cppreference.com/w/cpp/utility/tuple)类型 `_Tuple` ，它包含了可调用对象和参数的类型，这里使用了 [`decay_t`](https://zh.cppreference.com/w/cpp/types/decay) 来去除了类型的引用和 cv   限定。

3. `auto _Decay_copied = _STD make_unique<_Tuple>(_STD forward<_Fn>(_Fx), _STD forward<_Args>(_Ax)...)`

   - 使用 [`make_unique`](https://zh.cppreference.com/w/cpp/memory/unique_ptr/make_unique) 创建了一个独占指针，指向的是 `_Tuple` 类型的对象，**存储了传入的函数对象和参数的副本**。

4. `constexpr auto _Invoker_proc = _Get_invoke<_Tuple>(make_index_sequence<1 + sizeof...(_Args)>{})`

   - 调用 [`_Get_invoke`](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/thread#L65-L68) 函数，传入 `_Tuple` 类型和一个参数序列的索引序列（为了遍历形参包）。这个函数用于获取一个函数指针，指向了一个静态成员函数 [`_Invoke`](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/thread#L55-L63)，它是线程实际执行的函数。这两个函数都非常的简单，我们来看看：

    ```cpp
     template <class _Tuple, size_t... _Indices>
     _NODISCARD static constexpr auto _Get_invoke(index_sequence<_Indices...>) noexcept {
         return &_Invoke<_Tuple, _Indices...>;
     }
     
     template <class _Tuple, size_t... _Indices>
     static unsigned int __stdcall _Invoke(void* _RawVals) noexcept /* terminates */ {
         // adapt invoke of user's callable object to _beginthreadex's thread procedure
         const unique_ptr<_Tuple> _FnVals(static_cast<_Tuple*>(_RawVals));
         _Tuple& _Tup = *_FnVals.get(); // avoid ADL, handle incomplete types
         _STD invoke(_STD move(_STD get<_Indices>(_Tup))...);
         _Cnd_do_broadcast_at_thread_exit(); // TRANSITION, ABI
         return 0;
     }
    ```

   _Get_invoke 函数很简单，就是接受一个元组类型，和形参包的索引，传递给 _Invoke 静态成员函数模板，实例化，获取它的函数指针。

   > 它的形参类型我们不再过多介绍，你只需要知道 [`index_sequence`](https://en.cppreference.com/w/cpp/utility/integer_sequence)  这个东西可以用来接收一个由 `make_index_sequence` 创建的索引形参包，帮助我们进行遍历元组即可。[示例代码](https://godbolt.org/z/dv88aPGac)。

   **_Invoke 是重中之重，它是线程实际执行的函数**，如你所见它的形参类型是 `void*` ，这是必须的，要符合 `_beginthreadex` 执行函数的类型要求。虽然是 `void*`，但是我可以将它转换为 `_Tuple*` 类型，构造一个独占智能指针，然后调用 get() 成员函数获取底层指针，解引用指针，得到元组的引用初始化`_Tup` 。

   此时，我们就可以进行调用了，使用 [`std::invoke`](https://zh.cppreference.com/w/cpp/utility/functional/invoke) + `std::move`（默认移动） ，这里有一个形参包展开，`_STD get<_Indices>(_Tup))...`，_Tup 就是 std::tuple 的引用，我们使用 `std::get<>` 获取元组存储的数据，需要传入一个索引，这里就用到了 `_Indices`。展开之后，就等于 invoke 就接受了我们构造 std::thread 传入的可调用对象，调用可调用对象的参数，invoke 就可以执行了。

5. `_Thr._Hnd = reinterpret_cast<void*>(_CSTD _beginthreadex(nullptr, 0, _Invoker_proc, _Decay_copied.get(), 0, &_Thr._Id))`

   - 调用 [`_beginthreadex`](https://learn.microsoft.com/zh-cn/cpp/c-runtime-library/reference/beginthread-beginthreadex?view=msvc-170) 函数来启动一个线程，并将线程句柄存储到 `_Thr._Hnd` 中。传递给线程的参数为 `_Invoker_proc`（一个静态函数指针，就是我们前面讲的 **_Invoke**）和 `_Decay_copied.get()`（存储了函数对象和参数的副本的指针）。

6. `if (_Thr._Hnd) {`

   - 如果线程句柄 `_Thr._Hnd` 不为空，则表示线程已成功启动，将独占指针的所有权转移给线程。

7. `(void) _Decay_copied.release()`

   - 释放独占指针的所有权，因为已经将参数传递给了线程。

8. `} else { // failed to start thread`

   - 如果线程启动失败，则进入这个分支

9. `_Thr._Id = 0;`

   - 将线程ID设置为0。

10. `_Throw_Cpp_error(_RESOURCE_UNAVAILABLE_TRY_AGAIN);`

    - 抛出一个 C++ 错误，表示资源不可用，请再次尝试。

## 总结

需要注意，libstdc++ 和 libc++ 可能不同，就比如它们 64 位环境下 `sizeof(std::thread)` 的结果就可能是 **8**。libstdc++ 的实现只[保有一个 `std::thread::id`](https://github.com/gcc-mirror/gcc/blob/3e3d115c946944c81d8231dfbe778d4dae26cbb7/libstdc%2B%2B-v3/include/bits/std_thread.h#L123)。[参见](https://github.com/gcc-mirror/gcc/blob/3e3d115c946944c81d8231dfbe778d4dae26cbb7/libstdc%2B%2B-v3/include/bits/std_thread.h#L81-L85)。不过实测 gcc 不管是 `win32` 还是 `POSIX` 线程模型，线程对象的大小都是 8，宏 `_GLIBCXX_HAS_GTHREADS` 的值都为 1（[GThread](https://docs.gtk.org/glib/struct.Thread.html)）。

> ```cpp
>  class thread
>   {
>   public:
> #ifdef _GLIBCXX_HAS_GTHREADS
>     using native_handle_type = __gthread_t;
> #else
>     using native_handle_type = int;
> #endif
> ```
>
> `__gthread_t` 即 `void*`。

我们这里的源码解析涉及到的 C++ 技术很多，我们也没办法每一个都单独讲，那会显得文章很冗长，而且也不是重点。

相信你也感受到了，**不会模板，你阅读标准库源码，是无稽之谈**，市面上很多教程教学，教导一些实现容器，过度简化了，真要去出错了去看标准库的代码，那是不现实的。不需要模板的水平有多高，也不需要会什么元编程，但是基本的需求得能做到，得会，这里推荐一下：[**现代C++模板教程**](https://github.com/Mq-b/Modern-Cpp-templates-tutorial)。

---

学习完了也不要忘记了回答最初的问题：

1. **如何做到的默认按值复制？**

   `_Start` 的第一行代码展示了这一点。我们将传入的所有参数包装成一个元组类型，这些类型先经过 `decay_t` 处理，去除了引用与 cv 限定，自然就实现了默认复制。

   ```cpp
   using _Tuple                 = tuple<decay_t<_Fn>, decay_t<_Args>...>;
   ```

2. **为什么需要 `std::ref` ？**

   实现中将类型先经过 `decay` 处理，如果要传递引用，则必须用类包装一下才行，使用 `std::ref` 函数就会返回一个包装对象。

3. **如何支持只能移动的对象？**

   参数通过完美转发，最终调用时使用 `std::move`，这在线程实际执行的函数 `_Invoke` 中体现出来：

   ```cpp
   _STD invoke(_STD move(_STD get<_Indices>(_Tup))...);
   ```

4. 如何做到接受任意[可调用](https://zh.cppreference.com/w/cpp/named_req/Callable)对象？

   源码的实现很简单，主要是通过两层包装，最终将 `void*` 指针转换到原类型，然后使用 `std::invoke` 进行调用。

5. **如何创建的线程？**

   MSVC STL 调用 Win32 API `_beginthreadex` 创建线程；libstdc++ 调用 `__gthread_create` 函数创建线程，在 Windows 上实际上就是调用 `CreateThread`。
   `_beginthreadex` 和 `CreateThread` 都是微软提供的用于创建线程的 C 风格接口，它们的主要区别在于前者依赖于 C 运行时库，而后者更适合纯 Windows API 的情况。使用 `_beginthreadex` 可以确保正确初始化和清理 C 运行时库资源，而 `CreateThread` 则适用于不依赖于 C 运行时库的环境。

6. **传递参数一节中的：“*`std::thread` 内部会将保有的参数副本转换为右值表达式进行传递*”到底是如何做到的？**

   这就是第三个问题，差不多，无非是最后调用 `std::invoke` 函数之前，先 `std::move` 了。
