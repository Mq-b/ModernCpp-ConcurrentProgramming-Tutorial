# `st::async` 与 `std::future` 源码解析

## 前言

和之前一样的，我们以 MSVC STL 的实现进行讲解。

`std::future`，即未来体，是用来管理一个共享状态的类模板，用于等待关联任务的完成并获取其返回值。它自身不包含状态，需要通过如 `std::async` 之类的函数进行初始化。`std::async` 函数模板返回一个已经初始化且具有共享状态的 `std::future` 对象。因此，所有操作的开始应从 `std::async` 开始讲述。

> MSVC STL 很早之前就不支持 C++11 了，它的实现完全基于 **C++14**，出于某些原因 **C++17** 的一些库（如 [`invoke`](https://zh.cppreference.com/w/cpp/utility/functional/invoke)， _v 变量模板）被向后移植到了 **C++14** 模式，所以即使是 C++11 标准库设施，实现中可能也是使用到了 C++14、17 的东西。
>
> 注意，不用感到奇怪。

## 正式

```cpp
_EXPORT_STD template <class _Fty, class... _ArgTypes>
_NODISCARD_ASYNC future<_Invoke_result_t<decay_t<_Fty>, decay_t<_ArgTypes>...>> async(
    launch _Policy, _Fty&& _Fnarg, _ArgTypes&&... _Args) {
    // manages a callable object launched with supplied policy
    using _Ret   = _Invoke_result_t<decay_t<_Fty>, decay_t<_ArgTypes>...>;
    using _Ptype = typename _P_arg_type<_Ret>::type;
    _Promise<_Ptype> _Pr(
        _Get_associated_state<_Ret>(_Policy, _Fake_no_copy_callable_adapter<_Fty, _ArgTypes...>(
                                                 _STD forward<_Fty>(_Fnarg), _STD forward<_ArgTypes>(_Args)...)));

    return future<_Ret>(_From_raw_state_tag{}, _Pr._Get_state_for_future());
}

_EXPORT_STD template <class _Fty, class... _ArgTypes>
_NODISCARD_ASYNC future<_Invoke_result_t<decay_t<_Fty>, decay_t<_ArgTypes>...>> async(
    _Fty&& _Fnarg, _ArgTypes&&... _Args) {
    // manages a callable object launched with default policy
    return _STD async(launch::async | launch::deferred, _STD forward<_Fty>(_Fnarg), _STD forward<_ArgTypes>(_Args)...);
}
```

这段代码最直观的信息是，函数模板 `std::async` 有两个重载，其中第二个重载只是给了一个执行策略并将参数全部转发，调用第一个重载。也就是不指明执行策略的时候就会匹配到第二个重载版本。因此我们也只需要关注第二个版本了。

1. **模板参数和函数体外部信息**：

   - `_EXPOPT_STD` 是一个宏，当 `_BUILD_STD_MODULE` 宏定义且启用了 C++20 时，会被定义为 `export`，以便导出模块；否则它为空。

   - `_Fty` 表示可调用对象的类型。
   - `_ArgTypes` 是一个类型形参包，表示调用该可调用对象所需的参数类型。
   - `_NODISCARD_ASYNC` 是一个宏，表示属性 `[[nodiscard]]`，用于标记此函数的返回值不应被忽略。

2. 函数返回类型：

   ```cpp
   future<_Invoke_result_t<decay_t<_Fty>, decay_t<_ArgTypes>...>>
   ```

   虽然看起来复杂，但实际上是通过 `_Invoke_result_t` 获取可调用对象的返回类型。与标准库中的 [`std::invoke_result_t`](https://zh.cppreference.com/w/cpp/types/result_of) 基本相同。

   可以举一个使用 `std::invoke_result_t` 的例子：

   ```cpp
   template<class Fty, class... ArgTypes>
   std::future<std::invoke_result_t<std::decay_t<Fty>,std::decay_t<ArgTypes>...>>
   test_fun(Fty&& f,ArgTypes&&... args){
       return std::async(std::launch::async, std::forward<Fty>(f), std::forward<ArgTypes>(args)...);
   }
   
   auto result = test_fun([](int) {return 1; }, 1); // std::future<int>
   ```

   值得注意的是，所有类型在传递前都进行了 [`decay`](https://zh.cppreference.com/w/cpp/types/decay) 处理，也就是说不存在引用类型，是默认按值传递与 `std::thread` 的行为一致。

3. 函数形参：

   ```cpp
   async(launch _Policy, _Fty&& _Fnarg, _ArgTypes&&... _Args)
   ```

   `launch _Policy`: 表示任务的执行策略，可以是 `launch::async`（表示异步执行）或 `launch::deferred`（表示延迟执行），或者两者的组合。

   `_Fty&& _Fnarg`: 可调用对象，通过完美转发机制将其转发给实际的异步任务。

   `_ArgTypes&&... _Args`: 调用该可调用对象时所需的参数，同样通过完美转发机制进行转发。

4. `using _Ret   = _Invoke_result_t<decay_t<_Fty>, decay_t<_ArgTypes>...>;`

   `using _Ptype = typename _P_arg_type<_Ret>::type;`

   - 定义 `_Ret` 类型别名，它是使用 `_ArgTypes` 类型参数调用 `_Fty` 类型的可调用对象后得到的结果类型。也就是我们传入的可调用对象的返回类型；同样使用了 `_Invoke_result_t`（等价于  [`std::invoke_result_t`](https://zh.cppreference.com/w/cpp/types/result_of) ） 与 `decay_t`。

   - 其实 `_Ptype` 的定义确实在大多数情况下和 `_Ret` 是相同的，类模板 _P_arg_type 只是为了处理引用类型以及 void 的情况，参见 `_P_arg_type` 的实现：

     ```cpp
     template <class _Fret>
     struct _P_arg_type { // type for functions returning T
         using type = _Fret;
     };
     
     template <class _Fret>
     struct _P_arg_type<_Fret&> { // type for functions returning reference to T
         using type = _Fret*;
     };
     
     template <>
     struct _P_arg_type<void> { // type for functions returning void
         using type = int;
     };
     ```

     `_Ptype`：处理异步任务返回值的方式类型，它在语义上强调了异步任务返回值的处理方式，具有不同的实现逻辑和使用场景。在当前我们难以直接展示它的作用，不过可以推测，这个“`P`” 表示的是后文将使用的 `_Promise` 类模板。也就是说，定义 `_Ptype` 是为了配合 `_Promise` 的使用。我们将会在后文详细探讨 `_Promise` 类型的内部实现，并进一步解释 `_Ptype` 的具体作用。

5. `_Promise<_Ptype> _Pr`

   `_Promise` 类型本身不重要，很简单，关键还在于其存储的数据成员。

   ```cpp
   template <class _Ty>
   class _Promise {
   public:
       _Promise(_Associated_state<_Ty>* _State_ptr) noexcept : _State(_State_ptr, false), _Future_retrieved(false) {}
   
       _Promise(_Promise&&) = default;
   
       _Promise& operator=(_Promise&&) = default;
   
       void _Swap(_Promise& _Other) noexcept {
           _State._Swap(_Other._State);
           _STD swap(_Future_retrieved, _Other._Future_retrieved);
       }
   
       const _State_manager<_Ty>& _Get_state() const noexcept {
           return _State;
       }
       _State_manager<_Ty>& _Get_state() noexcept {
           return _State;
       }
   
       _State_manager<_Ty>& _Get_state_for_set() {
           if (!_State.valid()) {
               _Throw_future_error2(future_errc::no_state);
           }
   
           return _State;
       }
   
       _State_manager<_Ty>& _Get_state_for_future() {
           if (!_State.valid()) {
               _Throw_future_error2(future_errc::no_state);
           }
   
           if (_Future_retrieved) {
               _Throw_future_error2(future_errc::future_already_retrieved);
           }
   
           _Future_retrieved = true;
           return _State;
       }
   
       bool _Is_valid() const noexcept {
           return _State.valid();
       }
   
       bool _Is_ready() const noexcept {
           return _State._Is_ready();
       }
   
       _Promise(const _Promise&)            = delete;
       _Promise& operator=(const _Promise&) = delete;
   
   private:
       _State_manager<_Ty> _State;
       bool _Future_retrieved;
   };
   ```

   `_Promise` 类是对 `_State_manager` 类型的**包装**，并增加了一个表示状态的成员 `_Future_retrieved`。

   状态成员用于跟踪 `_Promise` 是否已经调用过 `_Get_state_for_future()` 成员函数；它默认为 `false`，在**第一次**调用 `_Get_state_for_future()` 成员函数时被置为 `true`，如果二次调用，就会抛出 [`future_errc::future_already_retrieved`](https://zh.cppreference.com/w/cpp/thread/future_errc) 异常。

   > 这类似于 `std::promise` 调用 [`get_future()`](https://zh.cppreference.com/w/cpp/thread/promise/get_future) 成员函数。[测试](https://godbolt.org/z/8anc9b3PT)。
   
   `_Promise` 的构造函数