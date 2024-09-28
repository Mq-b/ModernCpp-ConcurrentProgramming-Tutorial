# `std::scoped_lock` 的源码实现与解析

本单章专门介绍标准库在 C++17 引入的类模板 `std::scoped_lock` 的实现，让你对它再无疑问。

这会涉及到不少的模板技术，这没办法，就如同我们先前聊 [`std::thread` 的构造与源码分析](01thread的构造与源码解析.md)最后说的：“**不会模板，你阅读标准库源码，是无稽之谈**”。建议学习[现代C++模板教程](https://mq-b.github.io/Modern-Cpp-templates-tutorial/)。

我们还是一样的，以 MSVC STL 实现的 [`std::scoped_lock`](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/mutex#L476-L528) 代码进行讲解，不用担心，我们也查看了 [`libstdc++`](https://github.com/gcc-mirror/gcc/blob/7a01cc711f33530436712a5bfd18f8457a68ea1f/libstdc%2B%2B-v3/include/std/mutex#L743-L802) 、[`libc++`](https://github.com/llvm/llvm-project/blob/7ac7d418ac2b16fd44789dcf48e2b5d73de3e715/libcxx/include/mutex#L424-L488)的实现，并没有太多区别，更多的是一些风格上的。而且个人觉得 MSVC 的实现是最简单直观的。

## `std::scoped_lock` 的数据成员

`std::scoped_lock` 是一个类模板，它有两个特化，也就是有三个版本，其中的数据成员也是不同的。并且它们都不可移动不可复制，“*管理类*”应该如此。

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

std::scoped_lock<std::mutex> lc{ m1 };                   // 匹配到偏特化版本  保有一个 std::mutex&
std::scoped_lock<std::mutex, std::mutex> lc2{ m1,m2 };   // 匹配到主模板     保有一个 std::tuple<std::mutex&,std::mutex&>
std::scoped_lock<> lc3;                                 // 匹配到全特化版本  空
```

## `std::scoped_lock`的构造与析构

在上一节讲 `scoped_lock` 的数据成员的时候已经把这个模板类的全部源码，三个版本的代码都展示了，就不再重复。

这三个版本中，**只有两个版本需要介绍**，也就是

1. 形参包元素数量为一的偏特化，只管理一个互斥量的。
2. 主模板，可以管理任意个数的互斥量。

那这两个的共同点是什么呢？***构造上锁，析构解锁***。这很明显，明确这一点我们就开始讲吧。

---

```cpp
std::mutex m;
void f(){
    m.lock();
    std::lock_guard<std::mutex> lc{ m, std::adopt_lock };
}
void f2(){
    m.lock();
    std::scoped_lock<std::mutex>sp{ std::adopt_lock,m };
}
```

这段代码为你展示了 `std::lock_guard` 和 `std::scoped_lock` 形参包元素数量为一的偏特化的唯一区别：**调用不会上锁的构造函数的参数顺序不同**。那么到此也就够了。

接下来我们进入 `std::scoped_lock`  主模板的讲解：

```cpp
explicit scoped_lock(_Mutexes&... _Mtxes) : _MyMutexes(_Mtxes...) { // construct and lock
        _STD lock(_Mtxes...);
    }
```

这个构造函数做了两件事情，初始化数据成员 `_MyMutexes`让它保有这些互斥量的引用，以及给所有互斥量上锁，使用了 [`std::lock`](https://zh.cppreference.com/w/cpp/thread/lock) 帮助我们完成这件事情。

```cpp
explicit scoped_lock(adopt_lock_t, _Mutexes&... _Mtxes) noexcept // strengthened
    : _MyMutexes(_Mtxes...) {} // construct but don't lock
```

这个构造函数不上锁，只是初始化数据成员 `_MyMutexes`让它保有这些互斥量的引用。

```cpp
~scoped_lock() noexcept {
    _STD apply([](_Mutexes&... _Mtxes) { (..., (void) _Mtxes.unlock()); }, _MyMutexes);
}
```

析构函数就要稍微聊一下了，主要是用 [`std::apply`](https://zh.cppreference.com/w/cpp/utility/apply) 去遍历 [`std::tuple`](https://zh.cppreference.com/w/cpp/utility/tuple) ，让元组保有的互斥量引用都进行解锁。简单来说是 `std::apply` 可以将元组存储的参数全部拿出，用于调用这个可变参数的可调用对象，我们就能利用折叠表达式展开形参包并对其调用 `unlock()`。

> 不在乎其返回类型只用来实施它的副作用，显式转换为 `(void)` 也就是[*弃值表达式*](https://zh.cppreference.com/w/cpp/language/expressions#.E5.BC.83.E5.80.BC.E8.A1.A8.E8.BE.BE.E5.BC.8F)。在我们之前讲的 `std::thread` 源码中也有这种[用法](https://github.com/microsoft/STL/blob/8e2d724cc1072b4052b14d8c5f81a830b8f1d8cb/stl/inc/thread#L82)。
>
> 不过你可能有疑问：“我们的标准库的那些[互斥量](https://zh.cppreference.com/w/cpp/thread#.E4.BA.92.E6.96.A5) `unlock()` 返回类型都是 `void` 呀，为什么要这样？”
>
> 的确，这是个好问题，[libstdc++](https://github.com/gcc-mirror/gcc/blob/7a01cc711f33530436712a5bfd18f8457a68ea1f/libstdc%2B%2B-v3/include/std/mutex#L757-L758) 和 [libc++](https://github.com/llvm/llvm-project/blob/7ac7d418ac2b16fd44789dcf48e2b5d73de3e715/libcxx/include/mutex#L472-L475) 都没这样做，或许 MSVC STL 想着会有人设计的互斥量让它的 `unlock()` 返回类型不为 `void`，毕竟 [*互斥体* *(Mutex)*](https://zh.cppreference.com/w/cpp/named_req/Mutex) 没有要求 `unlock()` 的返回类型。

---

```cpp
template< class F, class Tuple >
constexpr decltype(auto) apply( F&& f, Tuple&& t );
```

这个函数模板接受两个参数，一个[*可调用* *(Callable)*](https://zh.cppreference.com/w/cpp/named_req/Callable)对象 f，以及一个元组 t，用做调用 f 。我们可以自己简单实现一下它，其实不算难，这种遍历元组的方式在之前讲 `std::thread` 的源码的时候也提到过。

```cpp
template<class Callable, class Tuple, std::size_t...index>
constexpr decltype(auto) Apply_impl(Callable&& obj,Tuple&& tuple,std::index_sequence<index...>){
    return std::invoke(std::forward<Callable>(obj), std::get<index>(std::forward<Tuple>(tuple))...);
}

template<class Callable, class Tuple>
constexpr decltype(auto) apply(Callable&& obj, Tuple&& tuple){
    return Apply_impl(std::forward<Callable>(obj), std::forward<Tuple>(tuple),
        std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
}
```

其实就是把元组给解包了，利用了 `std::index_sequence` + `std::make_index_sequence` 然后就用 `std::get` 形参包展开用 `std::invoke` 调用可调用对象即可，**非常经典的处理可变参数做法**，这个非常重要，一定要会使用。

举一个简单的调用例子：

```cpp
std::tuple<int, std::string, char> tuple{ 66,"😅",'c' };
::apply([](const auto&... t) { ((std::cout << t << ' '), ...); }, tuple);
```

> [运行测试](https://godbolt.org/z/n4aKo4xbr)。

使用了[折叠表达式](https://zh.cppreference.com/w/cpp/language/fold)展开形参包，打印了元组所有的元素。

## 总结

**如你所见，其实这很简单**。至少使用与了解其设计原理是很简单的。唯一的难度或许只有那点源码，处理可变参数，这会涉及不少模板技术，既常见也通用。还是那句话：“***不会模板，你阅读标准库源码，是无稽之谈***”。

相对于 `std::thread` 的源码解析，`std::scoped_lock` 还是简单的多。
