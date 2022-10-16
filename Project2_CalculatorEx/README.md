<div align="center">

# Project 2 - Calculator Ex

</div>

### 目录结构

|                                文件                                    |   备注   |
|------------------------------------------------------------------------|---------|
| [project.pdf](project.pdf)                                             | 题目 |
| **[report.pdf](report.pdf)**                                           | **报告**（上交） |
| [CMakeLists.txt](CMakeLists.txt)                                       | CMakeLists |
| [src/](src)                                                            | 源代码目录 |
| &emsp; [CMakeLists.txt](src/CMakeLists.txt)                            | CMakeLists |
| &emsp; [constant.cpp](src/constant.cpp), [constant.h](src/constant.h)  | 定义一些常数 |
| &emsp; [context.cpp](src/context.cpp), [context.h](src/context.h)      | 变量储存 |
| &emsp; [error.h](src/error.h)                                          | 自定义异常 |
| &emsp; [eval.cpp](src/eval.cpp), [eval.h](src/eval.h)                  | 一些数学函数（例如 sqrt）和数学方法 |
| &emsp; [main.cpp](src/main.cpp)                                        | 主程序入口点，主要交互逻辑 |
| &emsp; [node.cpp](src/node.cpp), [node.h](src/node.h)                  | AST 节点 |
| &emsp; [number.cpp](src/number.cpp), [number.h](src/number.h)          | 高精度数字 |
| &emsp; [parse.cpp](src/parse.cpp), [parse.h](src/parse.h)              | 解析（tokens → AST） |
| &emsp; [token.cpp](src/token.cpp), [token.h](src/token.h)              | tokenize（用户输入 → tokens） |
| [test/](test)                                                          | 测试文件目录 |
| &emsp; [CMakeLists.txt](test/CMakeLists.txt)                           | CMakeLists |
| &emsp; [test.hpp](test/test.hpp)                                       | 测试共用代码 |
| &emsp; [number_test.cpp](test/number_test.cpp), ...                    | 测试代码 |

### 主要功能

对于一些内建函数和运行选项，**可以用 `calc --help` 看帮助信息**。

这里展示一部分运行效果（`#` 后的内容为注释，实际运行需要去掉）。

#### 基本运算

```
> 5 * (200 + 30 / 7.5) - 1017 % 13
1017
> 99999999999999.123456789 + 100.86
100000000000099.983456789
> 66666666.666666 * 10171017.1017
678067806779993.2193219322
> 0.5 % 0.2   # 推广取模
0.1
```

#### 定义变量

```
> x = y = 3
3
> y = y * 2
6
> x + 2 * y
15
```

#### 数学函数 & 常数

```
> sqrt[2]
1.4142135623730950488
> ln[3]
1.0986122886681096914
> pow[2, 50]
1125899906842624
> pi
3.14159265358979323848
> e
2.71828182845904523536
```

我需要更高的精度！

```
$ ./calc --scale 100
> sqrt[2]
1.4142135623730950488016887242096980785696718753769480731766797379907324784621070388503875343276415727
> pi
3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342117068
> 
```

#### 定义函数

```
> f[x] = 3 * x * x - 5 * x + 13
> f[1.3]
11.57
> g[x] = f[f[x]]
> g[1.5]
401.9375
```

还能玩点大的：（其中 `if` 函数为内置的函数）

```
> fib[n] = if[n < 3, 1, fib[n - 1] + fib[n - 2]]
> fib[12]
144
```

还能玩点更大♂的，求 $f(x) = 0$ 的解：

```
> EPS = 1e-18
> abs[x] = if[ x < 0, 0 - x, x ]
> df[x] = (f[x + EPS] - f[x]) / EPS  # 尽管 f[x] 还没定义
> solver[x1, x2] = if[ abs[x1 - x2] < EPS, x1, solver[x1 - f[x1] / df[x1], x1] ]
>
> f[x] = x * x * x - 7
> solver[1, 0]
1.9129311827723891012
>
> f[x] = 3 * x * x * x - 2 * x + 7
> solver[1, 0]
-1.49311568009346432092
>
> f[x] = (1 - phi[x]) - 0.05         # 概统作业，第 49 页第 53 题 (c)
> solver[1, 0]
1.64485362695147271485
```

#### 列出变量

```
> env
(function) f = (((((3 * x) * x) * x) - (2 * x)) + 7)
(function) abs = if[(x < 0), (0 - x), x]
(function) solver = if[(abs[(x1 - x2)] < EPS), x1, solver[(x1 - (f[x1] / df[x1])), x1]]
(variable) EPS = 0.000000000000000001
(function) df = ((f[(x + EPS)] - f[x]) / EPS)
(function) if = <built-in function>
(function) sin = <built-in function>
(function) sqrt = <built-in function>
......  # 省略了一些 built-in function
```

#### 错误检测

啊，我手抖了，程序会崩吗？~~显然是不会的~~

```
> f[x] = h[x] + 10086
> f[7]
Input #0:
  f[x] = h[x] + 10086
         ~~~~
Error: no such variable or function: h
>
> g[x] = g[x + 1]
> g[7]
Input #2:
  g[x] = g[x + 1]
         ~~~~~~~~
Error: Your recursion depth is exceeded 5000
If you are sure what to do, you can disable the check with "--no_depth_check"
>
```

### 总结

我想摸鱼。
