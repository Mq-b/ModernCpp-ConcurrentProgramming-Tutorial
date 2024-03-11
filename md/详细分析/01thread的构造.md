# `std::thread` 的构造

我们这单章是为了专门解释一下 `std::thread` 是如何构造的，是如何创建线程传递参数的，让你彻底了解这个类。

我们以 **MSVC** 实现的 [`std::thread`](https://github.com/microsoft/STL/blob/main/stl/inc/thread) 代码进行讲解。

## `std::thread` 的数据成员

- **了解一个庞大的类，最简单的方式就是先看它的数据成员有什么**。

`std::thread` 只保有一个私有数据成员 [`_Thr`](https://github.com/microsoft/STL/blob/main/stl/inc/thread#L163)：

```cpp
private:
    _Thrd_t _Thr;
```

[`_Thrd_t`](https://github.com/microsoft/STL/blob/main/stl/inc/xthreads.h#L22-L26) 是一个结构体，它保有两个数据成员：

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

   [值初始化](https://zh.cppreference.com/w/cpp/language/value_initialization#:~:text=%E5%87%BD%E6%95%B0%E7%9A%84%E7%B1%BB%EF%BC%89%EF%BC%8C-,%E9%82%A3%E4%B9%88%E9%9B%B6%E5%88%9D%E5%A7%8B%E5%8C%96%E5%AF%B9%E8%B1%A1,-%EF%BC%8C%E7%84%B6%E5%90%8E%E5%A6%82%E6%9E%9C%E5%AE%83)了数据成员  _Thr ，这里的效果相当于给其成员 _Hnd 和 _Id 都进行[零初始化](https://zh.cppreference.com/w/cpp/language/zero_initialization)。

2. 移动构造函数，转移线程的所有权，构造 other 关联的执行线程的 `std::thread` 对象。此调用后 other 不再表示执行线程失去了线程的所有权。

   ```cpp
   thread(thread&& _Other) noexcept : _Thr(_STD exchange(_Other._Thr, {})) {}
   ```

   [_STD](https://github.com/microsoft/STL/blob/main/stl/inc/yvals_core.h#L1951) 是一个宏，展开就是 `::std::`，也就是 [`::std::exchange`](https://zh.cppreference.com/w/cpp/utility/exchange)，将 `_Other._Thr` 赋为 `{}` （也就是置空），返回 `_Other._Thr` 的旧值用以初始化当前对象的数据成员 `_Thr`。

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

前三个构造函数都没啥要特别聊的，非常简单，只有第四个构造函数较为复杂，且是我们本章重点，需要详细讲解。
