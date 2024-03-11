# `std::thread` 的构造

我们这单章其实是为了专门解释一下 `std::thread` 是如何构造的，是如何创建线程传递参数的，让你彻底了解这个类。

我们以 msvc 的 `std::thread` 的代码进行讲解。

## `std::thread` 的数据成员

- **了解一个庞大的类，最简单的方式就是先看它的数据成员有什么**。

`std::thread` 只保有一个私有数据成员：

```cpp
private:
    _Thrd_t _Thr;
```

`_Thrd_t` 是一个结构体，它保有两个数据成员：

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

`std::thread` 有四个构造函数，分别是：

1. 默认构造，构造不关联线程的新 std::thread 对象。

   ```cpp
   thread() noexcept;
   ```

2. 移动构造，转移线程的所有权，构造 other 关联的执行线程的 `std::thread` 对象。此调用后 other 不再表示执行线程失去了线程的所有权。

   ```cpp
   thread( thread&& other ) noexcept;
   ```
