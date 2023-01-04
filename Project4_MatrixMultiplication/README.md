<div align="center">

# Project 4 - Matrix Multiplication

</div>

### 目录结构

|                                文件                                    |   备注   |
|------------------------------------------------------------------------|---------|
| [project.pdf](project.pdf)                                           | 题目 |
| **[report.pdf](report.pdf)**                                         | **报告**（上交） |
| [CMakeLists.txt](CMakeLists.txt)                                     | CMakeLists |
| [src/](src)                                                          | 源代码目录 |
| &emsp; [CMakeLists.txt](src/CMakeLists.txt)                          | CMakeLists |
| &emsp; **[matrix.c](src/matrix.c)**, **[matrix.h](src/matrix.h)**    | **矩阵基础**（上交） |
| &emsp; **[mul.c](src/mul.c)**                                        | **矩阵乘法不同版本的实现**（上交） |
| [test/](test)                                                        | 测试文件目录 |
| &emsp; [CMakeLists.txt](test/CMakeLists.txt)                         | CMakeLists |
| &emsp; **[benchmark.cpp](test/benchmark.cpp)**                       | **性能测试代码**（上交） |

### mul.c 函数解释

|              函数名              |   备注   |
|----------------------------------|---------|
| `plain_gemm`                     | 基础暴力（循环顺序为 rck） |
| `plain_rkc_gemm`                 | 基础暴力（循环顺序为 rkc） |
| `plain_krc_gemm`                 | ~~基础暴力（循环顺序为 krc）~~ |
| `plain_kcr_gemm`                 | ~~基础暴力（循环顺序为 kcr）~~ |
| `plain_crk_gemm`                 | ~~基础暴力（循环顺序为 crk）~~ |
| `plain_ckr_gemm`                 | ~~基础暴力（循环顺序为 ckr）~~ |
| `plain_rkc_blocked_gemm`         | 暴力 rkc + 分块 |
| `plain_rkc_blocked_openmp_gemm`  | 上者 + OpenMP 并行 |
| `gepb_gemm`                      | 基本 GEPB 分块 |
| `gepb_packed_b_gemm`             | 上者 + 重排 B 矩阵 |
| `gepb_packed_a_gemm`             | 上者 + 重排 A 矩阵 |
| `gepb_final_gemm`                | 上者 + 重排 C 矩阵 |
| `gepb_aligned_gemm`              | 上者 + 告知编译器指针已对齐 |
| `gepb_parallel_gemm`             | 上上者 + 并行（pthread） |
| `openblas_gemm`                  | OpenBLAS |

### 碎碎念

唔，这次 Project 也没什么时间（虽然上个 Project 也是那么说），从星期四才开始做。  
不过我觉得矩阵优化的方法都比较地固定，所以可能按部就班就可以了，不用怎么思考。

但是只有开始了才发现事情往往变得十分尴尬，写完暴力之后，然后写调换循环顺序的时候，rkc 直接一骑绝尘，效率提高了近 10 倍。  
而且当我想要基于 rkc 进行进一步优化，例如循环展开、SIMD 化的时候，奇怪的事情发生了——效率不增反减！  
经过反编译二进制文件，发现因为 rkc 过于顺序访问，编译器直接进行了循环展开和 SIMD（AVX512）；
然后当我后面想要手动进行这些优化的时候，编译器就反而看不懂我写的东西了，生成出来的指令的质量就下降了，性能也下降了。  
所以我也很无语，你把我想要卷的都卷了，那我吃西北风去？我总不能写“将暴力调换顺序至 rkc，编译器自动循环展开、SIMD 化，report 完”吧？

不过需要说明的一点是，我这次 Project 都采用 `-march=native -O3` 编译。  
因为没人会在关心性能的场合不开 `-O3`，所以在不开 `-O3` 下讨论如何优化性能没有任何意义。  
而开了 `-O3` 的话，`register` 和 `i++` vs `++i` 肯定是不能卷了，现在编译器比我们还聪明。  
并且编译器还会尽全力生成更高效的指令，先进一点的编译器甚至还会自动向量化、循环展开。  
所以某种程度上，我们可能不是在写更高效的代码，而是写能让编译器看得懂的代码，帮助让它来生成更高效的指令。

那向量化（SIMD）、循环展开都不能卷了，那我们能卷什么呢？  
思考了很久之后，发现可以优化缓存，缓存也是影响性能的一个非常重要的方面。  
而且现在编译器也还不会贸然调整宏观运算顺序，所以这部分的工作还要我们来做。

对于如何观测缓存带来的影响，可以用 GFLOPS 衡量 CPU 一秒的 throughput（吞吐量）。  
然后将 n 从小到大步进，生成 GFLOPS-n 折线图，通过观察曲线，就可以一定程度上了解缓存带来的影响。

然后将 rkc 分块来优化缓存之后，性能确实又有提升。  
但是发现，接下来就不知道如何优化了。特别是如何分块，没有一个明确的思路。

不过好在在翻阅了 OpenBLAS 相关文档和细节之后，发现了 Goto 的一篇论文：Anatomy of High-Performance Matrix Multiplication，
然后用它的分块方法写了一个方法，然后在它基础上重排矩阵，让原本不连续的内存连续，一步一步优化。  
然后不负有心人，最终这个方案比 rkc 分块还要优秀，而且在 n 小于 1024 的时候，能达到 OpenBLAS 一半以上的效率。

---

然后因为没有显式地用平台相关的 AVX512 这些东西，所以代码可以直接搬到我的 MacBook M1 上，来跑 ARM 相关测试。  
发现上述方案的相对效率顺序没有变，但是 GFLOPS 曲线变得更平滑了，猜测是由于 M1 芯片是 SoC 芯片，L2 较大，而且访问内存的延迟较小。

在 MacBook M1 上，也可以测得与 Intel 平台上数量级相同的 GFLOPS（大概是 40 左右的样子），还是十分不错的。  
~~不过在报告中没有说明的一点是，在 M1 上，OpenBLAS 能跑到惊人的 GFLOPS 1000，不知道是不是动用了 M1 的神经网络核心，实在是太哈人了。~~

---

不过整体来说，在这个优化的过程，看着程序的效率一点一点提升，还是还是挺爽的（x）。

另：感谢 GitHub Codespaces 提供的支持 AVX512 的云开发环境，让我可以编译运行 AVX512 代码。以及可以提供 16 核心的环境来跑我的多线程代码。
