<div align="center">

# Project 3 - Matrix

</div>

### 目录结构

|                                文件                                    |   备注   |
|------------------------------------------------------------------------|---------|
| [project.pdf](project.pdf)                                           | 题目 |
| **[report.pdf](report.pdf)**                                         | **报告**（上交） |
| [CMakeLists.txt](CMakeLists.txt)                                     | CMakeLists |
| **[matrix.c](matrix.c)**, **[matrix.h](matrix.h)**                   | **核心源代码**（上交） |
| **[example.c](example.c)**                                           | **例子**（上交） |
| [Doxyfile.in](Doxyfile.in)                                           | Doxygen 配置 |
| [test/](test)                                                        | 测试文件目录 |
| &emsp; [CMakeLists.txt](test/CMakeLists.txt)                         | CMakeLists |
| &emsp; [test.h](test/test.h), [test.cpp](test/test.cpp)              | 测试共用函数 |
| &emsp; [test_allocate.cpp](test/test_allocate.cpp), ...              | 测试代码 |

### 主要 feature

- 基本功能：略
- 语义化参数：例如 `createMatrix(&A, (Size){2, 2});`
- 保存到文件，从文件加载
- 文档
- 单元测试（覆盖率大于 95%）

### 使用指南

#### 文档构建

用 Release 模式 `make` 之后，在构建目录下会生成 html 文件夹，打开其中的 index.html 就可以打开文档。  
当然也可以打开 latex 文件夹，编译生成 PDF 文档。

#### 覆盖率测试

运行 `make coverage`，就会自动运行测试，然后生成覆盖率报告。  
运行完之后，可以在构建目录下 test/coverage_report 中找到覆盖率报告，打开 index.html 即可查看。

### 总结

因为 ddl 刚好撞上迎新晚会，所以不是很有时间写，只能按照感觉随便瞎写了一点点。  
除了一些我有点想法的地方稍微花了点时间之外，其它地方我也只是完成了题目说的最基本的要求而已。特别是矩阵乘法，写了个暴力就跑路了，懒得优化了。

有些地方我也不是很满意，比如说矩阵分配与释放的部分，提供的 API 稍微有点多，也有点乱，可以适当删减一点。

说实话，这次真的卷不动了，report 也只是写了 7 页（不含目录）而已，求大佬放过我。

### 给后来人的建议

不要试图帮用户管理好所有事情，特别是不要为了检查野指针而追踪申请的每一块内存。库要做的事情就是把界限划分好，做好分内的事情，分外的事情就别管了，毕竟用户真的想搞事情你也阻止不了。

如果要提高易用性，就不要给用户过多选择，“假如有个小白用户用你的库，你觉得他能用对你的库吗？”（我也忘了是不是原话了，反正差不多是这个意思。）

头文件记得做好 header guard，这样重复 include 也不会炸。
另外为了让 C++ 用户也能正常使用这个库，头文件需要在 C++ 环境下加上 `extern "C"`（参考 [matrix.h#L8](https://github.com/YanWQ-monad/SUSTech_CS205_Projects/blob/master/Project3_Matrix/matrix.h#L7-L9)），记得闭合大括号。
（虽然后者于老师没说过，也不知道有没有加分，但是我觉得加上挺好的。）

另外可以参考于老师对于 Project 3 的建议：[BV1Vf4y1P7pq P52](https://www.bilibili.com/video/BV1Vf4y1P7pq?p=52)。
为了方便大家偷懒，我复制了一些关键的在下面，然后还夹杂着一些个人见解。

- 用 `size_t` 类型表示矩阵大小
- 使用一维数组 `float*` 储存数据
- 记得做参数检查（`mat` 本身以及 `mat->data` 两个都要检查）
- 管理好内存（特别是错误发生的时候不要忘记释放内存）
- 循环中使用 `r`, `c` 表示行和列，而不是 `i`, `j`
- 使用 `memcpy` 来 copy 数据
