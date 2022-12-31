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
