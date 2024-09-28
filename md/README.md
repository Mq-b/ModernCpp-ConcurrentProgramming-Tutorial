# 阅读须知

&emsp;&emsp;本套教程侧重点在于使用 C++ 并发支持库进行多线程编程。我们假设读者最低水平为：C++11 + STL + template，可能没有接触过 C++ 标准并发库，假设略微了解操作系统基本知识。

&emsp;&emsp;我们强调了模板，因为并发支持库的很多设施其实现是较为简单的，概念与使用，再结合源码讲解会更加简单直观，然而要想阅读学习源码，模板的知识必不可少。不需要模板的水平有多高，也不需要会什么元编程，但是基本的需求得能做到，得会，这里推荐一下：[**《现代C++模板教程》**](https://github.com/Mq-b/Modern-Cpp-templates-tutorial)。

&emsp;&emsp;本教程不保证你学习之后的成果，不过依然可以自信地说：**本教程在中文社区的同类型教程中是绝对的第一**。事实上只需要一句话就可以表达了——**伟大无需多言**。

## 学习注意事项

&emsp;&emsp;我们的教程中常包含许多外部链接，这并非当前描述不足或者不够严谨，而是为了考虑读者的水平和可能的扩展学习需求。同时，也希望能让读者避免获取二手知识与理解，我们提供的链接基本都是较为专业的文档或官方网站。

&emsp;&emsp;虽然教程名为《现代 C++ 并发编程教程》，但我们也扩展涉及了许多其他知识，包括但不限于：Win32、POSIX API；MSVC STL、libstdc++、libc++ 对标准库的实现；GCC 与 MSVC 的编译器扩展，以及 Clang 对它们的兼容；使用 CMake + Qt 构建带 UI 的程序，展示多线程异步的必要性；不同架构的内存模型（例如 x86 架构内存模型：Total Store Order (TSO)，较为严格的内存模型）。

&emsp;&emsp;既然强调了“**现代**”，那自然是全方面的，具体的读者会在学习中感受到的。

&emsp;&emsp;另外我们的代码都会测试三大编译器 `Clang`、`GCC`、`MSVC`。通常都会是最新的，`Clang18`、`GCC14`。我们的教程中常常会提供 [Complier Explorer](https://godbolt.org/) 的运行测试链接以确保正确性，以及方便读者的测试与学习。如果你对此网站的使用不熟悉，可以阅读[使用文档](https://mq-b.github.io/Loser-HomeWork/src/%E5%8D%A2%E7%91%9F%E6%97%A5%E7%BB%8F/godbolt%E4%BD%BF%E7%94%A8%E6%96%87%E6%A1%A3)。

## 代码风格

&emsp;&emsp;我们的代码风格较为简洁明了，命名全部使用下划线连接，而不是驼峰命名法。花括号通常只占一行，简短的代码可以不额外占行。一般初始化时使用 `{}`，而非 `()` 或者 `=` 。这样简单直观，避免歧义和许多问题。`#include` 引入头文件时需要在尖括号或引号前后加空格。

```cpp
#include <iostream>

struct move_only{
    move_only() { std::puts("默认构造"); }
    move_only(move_only&&)noexcept { std::puts("移动构造"); }
    move_only& operator=(move_only&&) noexcept {
        std::puts("移动赋值");
        return *this;
    }
    move_only(const move_only&) = delete;
};

int main(){
    move_only m{};
    char buffer[1024]{} // 全部初始化为 0
}
```

如果是标量类型，可能考虑使用复制初始化，而非 `{}`，如：`int n = 0;`。

## 总结

&emsp;&emsp;本教程长期维护，接受 pr 与 issue。

&emsp;&emsp;好了，稍微了解了一下，我们可以开始进入正式的学习内容了。
