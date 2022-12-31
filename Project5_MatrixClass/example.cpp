#include <iostream>
using namespace std;

#include "matrix.hpp"

int main() {
    // 定义并初始化两个矩阵
    Mat<double> m1({{3., 4.}, {8., 7.}});
    Mat<double> m2({{7., 2.}, {4., 9.}});
    // Mat<double> m3({{7., 2.}, {4., 9., 7.}});  // 这个会编译错误（行不等长）

    cout << m1 * m2 << endl;  // 矩阵乘法
    cout << m1 * 2. << endl;  // 标量乘法
    cout << 2. * m1 << endl;  // 反过来也可以
    cout << (m1 < m2) << endl;  // 比较每个元素相对大小（返回矩阵）
    m1 -= 10;  // 标量减法

    cout << m1.min() << endl;  // 求最小值（标量）
    cout << m1.abs() << endl;  // 求绝对值（矩阵）

    cout << m1.transpose() << endl;  // no hard copy
    m2.transpose() = m1;
    cout << m2 << endl;  // m2 应该会等于 m1 的转置

    constexpr double EPS = 1E-6;
    cout << (m2 == m1.transpose()).all() << endl;  // 直接比较（可能受误差影响）
    cout << ((m2 - m1.transpose()).abs().max() < EPS) << endl;  // 基于 EPS 的比较

    m1.block(Rect({0, 0}, {1, 2})) = m2.block(Rect({1, 0}, {1, 2}));  // 拷贝 ROI
    cout << m1 << endl;


    Mat<int, 2> A(2, 2);  // 双通道 2x2 矩阵
    A.channel(0) = m1.cast<int>();  // 将通道 0 赋值为 m1（转成 int）
    A.channel(1).set({{4, 5}, {6, 7}});
    cout << A.channel(0) % A.channel(1) << endl;
    cout << A << endl;

    return 0;
}
