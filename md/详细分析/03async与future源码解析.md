# `st::async` 与 `std::future` 源码解析

## 前言

和之前一样的，我们以 MSVC STL 的实现进行讲解。

`std::future`，即未来体，是用来管理一个共享状态的类模板，用于等待关联任务的完成并获取其返回值。它自身不包含状态，需要通过如 `std::async` 之类的函数进行初始化。`std::async` 函数模板返回一个已经初始化且具有共享状态的 `std::future` 对象。

因此，所有操作的开始应从 `std::async` 开始讲述。

需要注意的是，它们的实现彼此之间会共用不少设施，在讲述 `std::async` 源码的时候，对于 `std::future` 的内容同样重要。

> MSVC STL 很早之前就不支持 C++11 了，它的实现完全基于 **C++14**，出于某些原因 **C++17** 的一些库（如 [`invoke`](https://zh.cppreference.com/w/cpp/utility/functional/invoke)， _v 变量模板）被向后移植到了 **C++14** 模式，所以即使是 C++11 标准库设施，实现中可能也是使用到了 C++14、17 的东西。
>
> 注意，不用感到奇怪。

## `std::async`

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

   [`_Promise`](https://github.com/microsoft/STL/blob/f54203f/stl/inc/future#L999-L1055) 类型本身不重要，很简单，关键还在于其存储的数据成员。

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

   `_Promise` 类模板是对 **[`_State_manager`](https://github.com/microsoft/STL/blob/f54203f/stl/inc/future#L688-L825) 类模板的包装**，并增加了一个表示状态的成员 `_Future_retrieved`。

   状态成员用于跟踪 `_Promise` 是否已经调用过 `_Get_state_for_future()` 成员函数；它默认为 `false`，在**第一次**调用 `_Get_state_for_future()` 成员函数时被置为 `true`，如果二次调用，就会抛出 [`future_errc::future_already_retrieved`](https://zh.cppreference.com/w/cpp/thread/future_errc) 异常。

   > 这类似于 `std::promise` 调用 [`get_future()`](https://zh.cppreference.com/w/cpp/thread/promise/get_future) 成员函数。[测试](https://godbolt.org/z/8anc9b3PT)。

   `_Promise` 的构造函数接受的却不是 `_State_manager` 类型的对象，而是 `_Associated_state` 类型的指针，用来初始化数据成员 `_State`。

   ```cpp
   _State(_State_ptr, false)
   ```

   这是因为实际上 `_State_manager` 类型的[实现](https://github.com/microsoft/STL/blob/f54203f/stl/inc/future#L822-L824)就是保有了 [`Associated_state`](https://github.com/microsoft/STL/blob/f54203f/stl/inc/future#L198-L445) 指针，以及一个状态成员：

   ```cpp
   private:
       _Associated_state<_Ty>* _Assoc_state = nullptr;
       bool _Get_only_once                  = false;
   ```

   也可以简单理解 `_State_manager` 又是对 `Associated_state` 的包装，其中的大部分接口实际上是调用 `_Assoc_state` 的成员函数，如：

   ```cpp
   void wait() const { // wait for signal
       if (!valid()) {
           _Throw_future_error2(future_errc::no_state);
       }
   
       _Assoc_state->_Wait();
   }
   ```

   - **一切的重点，最终在 `Associated_state` 上**。

   然而它也是最为复杂的，我们在讲 `std::thread`-构造源码解析 中提到过一句话：

   > - **了解一个庞大的类，最简单的方式就是先看它的数据成员有什么**。

   ```cpp
   public:
       conditional_t<is_default_constructible_v<_Ty>, _Ty, _Result_holder<_Ty>> _Result;
       exception_ptr _Exception;
       mutex _Mtx;
       condition_variable _Cond;
       bool _Retrieved;
       int _Ready;
       bool _Ready_at_thread_exit; // TRANSITION, ABI
       bool _Has_stored_result;
       bool _Running;
   ```

    这是 `Associated_state` 的数据成员，其中有许多的 `bool` 类型的状态成员，同时最为明显重要的三个设施是：**异常指针**、**互斥量**、**条件变量**。

   根据这些数据成员我们就能很轻松的猜测出 `Associated_state` 模板类的作用和工作方式。

   - **异常指针**：用于存储异步任务中可能发生的异常，以便在调用 `future::get` 时能够重新抛出异常。
   - **互斥量和条件变量**：用于在异步任务和等待任务之间进行同步。当异步任务完成时，条件变量会通知等待的任务。

   **`_Associated_state` 模板类负责管理异步任务的状态，包括结果的存储、异常的处理以及任务完成的通知。它是实现 `std::future` 和 `std::promise` 的核心组件之一**，通过 `_State_manager` 和 `_Promise` 类模板对其进行封装和管理，提供更高级别的接口和功能。

   ```plaintext
   +---------------------+
   |    _Promise<_Ty>    |
   |---------------------|
   | - _State            | -----> +---------------------+
   | - _Future_retrieved |       |  _State_manager<_Ty> |
   +---------------------+       |----------------------|
                                 | - _Assoc_state       | -----> +-------------------------+
                                 | - _Get_only_once     |       | _Associated_state<_Ty>   |
                                 +----------------------+       +-------------------------+
   ```

   上图是 `_Promise`、_`State_manager`、`_Associated_state` 之间的**包含关系示意图**，理解这个关系对我们后面**非常重要**。

   到此就可以了，我们不需要在此处就详细介绍这三个类，但是你需要大概的看一下，这非常重要。

6. 初始化 `_Promie` 对象：

   `_Get_associated_state<_Ret>(_Policy, _Fake_no_copy_callable_adapter<_Fty, _ArgTypes...>(_STD forward<_Fty>(_Fnarg), _STD forward<_ArgTypes>(_Args)...))`

   很明显，这是一个函数调用，将我们 `std::async` 的参数全部转发给它，它是重要而直观的。

   `_Get_associated_state` 函数根据启动模式（`launch`）来决定创建的异步任务状态对象类型：

   ```cpp
   template <class _Ret, class _Fty>
   _Associated_state<typename _P_arg_type<_Ret>::type>* _Get_associated_state(launch _Psync, _Fty&& _Fnarg) {
       // construct associated asynchronous state object for the launch type
       switch (_Psync) { // select launch type
       case launch::deferred:
           return new _Deferred_async_state<_Ret>(_STD forward<_Fty>(_Fnarg));
       case launch::async: // TRANSITION, fixed in vMajorNext, should create a new thread here
       default:
           return new _Task_async_state<_Ret>(_STD forward<_Fty>(_Fnarg));
       }
   }
   ```

   `_Get_associated_state` 函数返回一个 `_Associated_state` 指针，该指针指向一个新的 `_Deferred_async_state` 或 `_Task_async_state` 对象。这两个类分别对应于异步任务的两种不同执行策略：**延迟执行**和**异步执行**。

   > 这段代码也很好的说明在 MSVC STL 中，`launch::async | launch::deferred` 和 `launch::async` 的行为是相同的，即都是异步执行。

   ---

   **`_Task_async_state` 类型**

   ```cpp
   template <class _Rx>
   class _Task_async_state : public _Packaged_state<_Rx()>
   ```

   `_Task_async_state` 是 `_Packaged_state` 的派生类，用于异步执行任务。它的构造函数接受一个函数对象，并将其传递给基类 `_Packaged_state` 的构造函数。

   `_Packaged_state` 类型只有一个数据成员 `std::function` 类型的对象 `_Fn`，它表示需要执行的异步任务，而它又继承自 _Associated_state。

   ```cpp
   template <class _Ret, class... _ArgTypes>
   class _Packaged_state<_Ret(_ArgTypes...)>
       : public _Associated_state<_Ret>
   ```

   我们直接看 `_Task_async_state` 类型的构造函数实现即可：

   ```cpp
   template <class _Fty2>
   _Task_async_state(_Fty2&& _Fnarg) : _Mybase(_STD forward<_Fty2>(_Fnarg)) {
       _Task = ::Concurrency::create_task([this]() { // do it now
           this->_Call_immediate();
       });
   
       this->_Running = true;
   }
   ```

   它的数据成员：

   ```cpp
   private:
       ::Concurrency::task<void> _Task;
   ```

   这里其实使用到了微软自己实现的 [并行模式库](https://learn.microsoft.com/zh-cn/cpp/parallel/concrt/parallel-patterns-library-ppl?view=msvc-170)（PPL），简而言之 `async` 策略并不是单纯的创建线程让任务执行，而是使用了微软的 `::Concurrency::create_task` ，它从**线程池**中获取线程并执行任务返回包装对象。

   `this->_Call_immediate();` 是调用的父类 `_Packaged_state` 的成员函数：

   ```cpp
   void _Call_immediate(_ArgTypes... _Args) { // call function object
       _TRY_BEGIN
       // call function object and catch exceptions
       this->_Set_value(_Fn(_STD forward<_ArgTypes>(_Args)...), false);
       _CATCH_ALL
       // function object threw exception; record result
       this->_Set_exception(_STD current_exception(), false);
       _CATCH_END
   }
   ```

   它则调用了 `_Associated_state` 的成员函数（`_Set_value`、`_set_exception`），传递的可调用对象执行结果，以及可能的异常，将结果或异常存储在 `_Associated_state` 中。
