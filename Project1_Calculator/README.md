<div align="center">

# Project 1 - Calculator

</div>

### 文件列表

|                     文件                           |   备注   |
|----------------------------------------------------|---------|
| [project.pdf](project.pdf)                         | 题目 |
| **[mul.cpp](mul.cpp)**                             | **主程序**（上交） |
| **[report.pdf](report.pdf)**                       | **报告**（上交） |
| [CMakeLists.txt](CMakeLists.txt)                   | cmake 用 |
| [mul_test.cpp](mul_test.cpp)                       | 单元测试 |
| [mul_benchmark.cpp](mul_benchmark.cpp)             | 性能测试 |
| [abi.cpp](abi.cpp)                                 | 给 Python 用的接口 |
| [correctness_test.py](correctness_test.py)         | Python 对拍器 |
| [mul_alternative_1.cpp](mul_alternative_1.cpp)     | report.pdf 附件 |
| [mul_alternative_2.cpp](mul_alternative_2.cpp)     | 同上 |

虽然这次 Project 只要求上交一个源文件，但是为了开发编写方便，就…顺手加了点文件来方便自己。

### 如何运行

本项目使用了 C++17 标准，所以在编译时请确保加上了 `-std=c++17` 选项。

对于基本运行，把 `mul.cpp` 编译了就能用了，其它文件都可以不管。

如果你想要进一步了解这个项目，那么可能需要学习如何使用 CMake。不过不会的话也不复杂，先新建个目录 `build`，
然后在 `build` 目录中执行 `cmake -DCMAKE_BUILD_TYPE=Release ..`，然后再执行 `make` 即可。

构建好了之后就会生成一些二进制文件，下面列出一些测试和运行的操作：

- `./mul`：运行主程序
- `ctest`：运行所有测试（性能测试除外）
- `./mul_test`：运行单元测试
- `python3 correctness_test.py`：运行对拍（当然可以修改源代码来达到持续对拍的目的）
- `./mul_benchmark`：运行性能测试
- `./mul_alternative_1`/`2`：运行附件

### 碎碎念

说实话，这个项目，不严谨地说，是我第二次写。

上学期当 Java SA 的时候，第 4 次 Assignment 就是写一个高精度二进制乘法器（加减法也有，这里就不提了）。
当时也听说有学长吐槽，这个作业题属于是 SA 学了什么，反手就出成 Assignment，就比如那次的高精度乘法，
就是他们某个课程刚好要做一个高精度的项目，然后转手就出给 Java A 的学生做了。就，诶，我也不知道怎么评价。

啊扯远了，刚刚说到上学期 Java 出过一个高精度二进制乘法器，然后我当时作为 SA，还写了两种实现，一种是基于压位的，
因为数字也不长，$O(\frac{n^2}{32^2})$ 能过。然后另外一种就是没有压位，写了一个 FFT，我在 Lab 课上讲了思路，代码也放到 GitHub 了：
https://github.com/YanWQ-monad/SUSTech_CS102A_2022S_Answer/blob/master/Assignment4/BigBinary2.java#L94 。
不知道当时选了张锋巍老师的 Lab 课的同学，听完我的课之后，有没有真的事后去了解 FFT，并且在这次项目中用上呢（笑）。
~~如果没有的话赶紧看 report 复习一下吧（笑）。~~

不过话说回来，我没有在吃老本哈\~。这次 Project 把 FFT 和压位结合了起来，发现确实能擦出火花，还是蛮好玩的。
