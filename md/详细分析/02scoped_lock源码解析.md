# `std::scoped_lock` 的源码实现与解析

本单章专门介绍标准库在 C++17 引入的类模板 `std::scoped_lock` 的实现，让你对它再无疑问。

这会涉及到不少的模板技术，这没办法，就如同我们先前聊 [`std::thread` 的构造与源码分析](01thread的构造与源码解析.md)最后说的：“**不会模板，你阅读标准库源码，是无稽之谈**”。

我们还是一样的，以 MSVC STL 的实现的 [`std::scoped_lock`](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/mutex#L476-L528) 代码进行讲解，不用担心，我们也查看了 [`libstdc++`](https://github.com/gcc-mirror/gcc/blob/7a01cc711f33530436712a5bfd18f8457a68ea1f/libstdc%2B%2B-v3/include/std/mutex#L743-L802) 、[`libc++`](https://github.com/llvm/llvm-project/blob/7ac7d418ac2b16fd44789dcf48e2b5d73de3e715/libcxx/include/mutex#L424-L488)的实现，并没有太多区别，更多的是一些风格上的。而且个人觉得 MSVC 的实现是最简单直观的。

## `std:scoped_lock` 的数据成员

`std::scoped_lock` 是一个类模板，它有两个特化，也就是有三个版本，其中的数据成员也是不同的。

1. 主模板，是一个可变参数类模板，声明了一个类型形参包 `_Mutexes`，**存储了一个 `std::tuple`**，具体类型根据类型形参包决定。

   ```cpp
   _EXPORT_STD template <class... _Mutexes>
   class _NODISCARD_LOCK scoped_lock { // class with destructor that unlocks mutexes
   public:
       explicit scoped_lock(_Mutexes&... _Mtxes) : _MyMutexes(_Mtxes...) { // construct and lock
           _STD lock(_Mtxes...);
       }
   
       explicit scoped_lock(adopt_lock_t, _Mutexes&... _Mtxes) noexcept // strengthened
           : _MyMutexes(_Mtxes...) {} // construct but don't lock
   
       ~scoped_lock() noexcept {
           _STD apply([](_Mutexes&... _Mtxes) { (..., (void) _Mtxes.unlock()); }, _MyMutexes);
       }
   
       scoped_lock(const scoped_lock&)            = delete;
       scoped_lock& operator=(const scoped_lock&) = delete;
   
   private:
       tuple<_Mutexes&...> _MyMutexes;
   };
   ```

2. 对模板类型形参包只有一个类型情况的**偏特化**，是不是很熟悉，和 `lock_guard` 几乎没有任何区别，**保有一个互斥量的引用**，构造上锁，析构解锁，提供一个额外的构造函数让构造的时候不上锁。所以用 `scoped_lock` 替代 `lock_guard` 不会造成任何额外开销。

   ```cpp
   template <class _Mutex>
   class _NODISCARD_LOCK scoped_lock<_Mutex> {
   public:
       using mutex_type = _Mutex;
   
       explicit scoped_lock(_Mutex& _Mtx) : _MyMutex(_Mtx) { // construct and lock
           _MyMutex.lock();
       }
   
       explicit scoped_lock(adopt_lock_t, _Mutex& _Mtx) noexcept // strengthened
           : _MyMutex(_Mtx) {} // construct but don't lock
   
       ~scoped_lock() noexcept {
           _MyMutex.unlock();
       }
   
       scoped_lock(const scoped_lock&)            = delete;
       scoped_lock& operator=(const scoped_lock&) = delete;
   
   private:
       _Mutex& _MyMutex;
   };
   ```

3. 对类型形参包为空的情况的全特化，**没有数据成员**。

   ```cpp
   template <>
   class scoped_lock<> {
   public:
       explicit scoped_lock() = default;
       explicit scoped_lock(adopt_lock_t) noexcept /* strengthened */ {}
   
       scoped_lock(const scoped_lock&)            = delete;
       scoped_lock& operator=(const scoped_lock&) = delete;
   };
   ```

---

```cpp
std::mutex m1,m2;

std::scoped_lock<std::mutex>lc{ m1 };                   // 匹配到偏特化版本  保有一个 std::mutex&
std::scoped_lock<std::mutex, std::mutex>lc2{ m1,m2 };   // 匹配到主模板     保有一个 std::tuple<std::mutex&,std::mutex&>
std::scoped_lock<> lc3;                                 // 匹配到全特化版本  空
```

## `std:scoped_lock`的构造与析构

在上一节讲 `scoped_lock` 的数据成员的时候已经把这个模板类的全部源码，三个版本的代码都展示了，就不再重复。
