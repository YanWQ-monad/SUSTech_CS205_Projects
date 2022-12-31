#ifndef _MATRIX_CLASS_H_
#define _MATRIX_CLASS_H_

#include <cstddef>
#include <limits>
#include <memory>
#include <sstream>

#include <cblas.h>

template<typename T>
struct binary_assignment {
    constexpr T operator()(T, T rhs) const {
        return rhs;
    }
};

class Size {
public:
    size_t rows, cols;

    Size(size_t rows, size_t cols) : rows(rows), cols(cols) {}

    bool operator==(const Size &rhs) const {
        return rows == rhs.rows && cols == rhs.cols;
    }

    bool operator!=(const Size &rhs) const {
        return !(rhs == *this);
    }
};

class Point {
public:
    size_t r, c;

    Point(size_t r, size_t c) : r(r), c(c) {}
};

class Rect {
public:
    size_t r, c, rows, cols;

    Rect(Point top_left, Size size) : r(top_left.r), c(top_left.c), rows(size.rows), cols(size.cols) {}
    Rect(size_t r, size_t c, size_t rows, size_t cols) : r(r), c(c), rows(rows), cols(cols) {}
};

template<typename T, size_t CHANNELS, typename Derived>
class MatBase;

template<typename T, size_t CHANNELS = 1>
class Mat;

template<typename T, size_t CHANNELS>
class MatChannelProxyConst;

template<typename T, size_t CHANNELS>
class MatChannelProxy;

template<typename T, size_t CHANNELS>
class MatTransposeProxy;

template<typename T, size_t CHANNELS, typename Derived>
class MatBase {
public:
    [[nodiscard]] inline const Derived& derived() const { return *static_cast<const Derived*>(this); }
    [[nodiscard]] inline Derived& derived() { return *static_cast<Derived*>(this); }

    [[nodiscard]] inline size_t rows() const { return derived().rows(); }
    [[nodiscard]] inline size_t cols() const { return derived().cols(); }
    [[nodiscard]] inline Size size() const { return derived().size(); }

    [[nodiscard]] Mat<T, CHANNELS> eval() const {
        Mat<T, CHANNELS> mat(size());
        return mat = *this;
    }

    template<typename U>
    [[nodiscard]] Mat<U, CHANNELS> cast() const {
        Mat<U, CHANNELS> mat(rows(), cols());
        for (size_t r = 0; r < rows(); r++)
            for (size_t c = 0; c < cols(); c++)
                for (size_t l = 0; l < CHANNELS; l++)
                    mat(r, c, l) = operator()(r, c, l);
        return mat;
    }

    inline T operator()(size_t r, size_t c, size_t channel = 0) const {
        return derived()(r, c, channel);
    }

    inline T& operator()(size_t r, size_t c, size_t channel = 0) {
        return derived()(r, c, channel);
    }

    T at(size_t r, size_t c, size_t channel = 0) const {
        if (r >= rows() || c >= cols() || channel >= CHANNELS)
            throw std::out_of_range("index out of range");
        return derived()(r, c, channel);
    }

    T& at(size_t r, size_t c, size_t channel = 0) {
        if (r >= rows() || c >= cols() || channel >= CHANNELS)
            throw std::out_of_range("index out of range");
        return derived()(r, c, channel);
    }

    template<size_t M, size_t N>
    void set(const T(&list)[M][N]) {
        static_assert(CHANNELS == 1 && "2D array only support single channel, use 3D array instead");
        if (M != rows() || N != cols())
            throw std::invalid_argument("matrix size mismatched");

        for (size_t r = 0; r < rows(); r++)
            for (size_t c = 0; c < cols(); c++)
                operator()(r, c) = list[r][c];
    }

    template<size_t M, size_t N>
    void set(const T(&list)[M][N][CHANNELS]) {
        if (M != rows() || N != cols())
            throw std::invalid_argument("matrix size mismatched");

        for (size_t r = 0; r < rows(); r++)
            for (size_t c = 0; c < cols(); c++)
                for (size_t l = 0; l < CHANNELS; l++)
                    operator()(r, c, l) = list[r][c][l];
    }

//    template<typename OtherDerived>
//    Derived& operator=(const MatBase<T, CHANNELS, OtherDerived> &rhs) {
//        call_piecewise_op(*this, rhs, binary_assignment<T>());
//        return derived();
//    }

    template<typename OtherDerived>
    Derived& operator+=(const MatBase<T, CHANNELS, OtherDerived> &rhs) {
        call_piecewise_op(*this, rhs, std::plus<T>());
        return derived();
    }

    Derived& operator+=(const T rhs) {
        call_scalar_op(*this, rhs, std::plus<T>());
        return derived();
    }

    template<typename U>
    Mat<T, CHANNELS> operator+(const U &rhs) const {
        return eval() += rhs;
    }

    template<typename OtherDerived>
    Derived& operator-=(const MatBase<T, CHANNELS, OtherDerived> &rhs) {
        call_piecewise_op(*this, rhs, std::minus<T>());
        return derived();
    }

    Derived& operator-=(const T rhs) {
        call_scalar_op(*this, rhs, std::minus<T>());
        return derived();
    }

    template<typename U>
    Mat<T, CHANNELS> operator-(const U &rhs) const {
        return eval() -= rhs;
    }

    Mat<T, CHANNELS> operator-() const {
        Mat<T, CHANNELS> result = eval();
        call_self_piecewise_op(result, std::negate<T>());
        return result;
    }

    Derived& operator*=(const T rhs) {
        call_scalar_op(*this, rhs, std::multiplies<T>());
        return derived();
    }

    Mat<T, CHANNELS> operator*(const T rhs) const {
        return eval() *= rhs;
    }

    template<typename OtherDerived>
    Derived& operator/=(const MatBase<T, CHANNELS, OtherDerived> &rhs) {
        call_piecewise_op(*this, rhs, std::divides<T>());
        return derived();
    }

    Derived& operator/=(const T rhs) {
        call_scalar_op(*this, rhs, std::divides<T>());
        return derived();
    }

    template<typename U>
    Mat<T, CHANNELS> operator/(const U &rhs) const {
        return eval() /= rhs;
    }

    template<typename OtherDerived>
    Derived& operator%=(const MatBase<T, CHANNELS, OtherDerived> &rhs) {
        call_piecewise_op(*this, rhs, std::modulus<T>());
        return derived();
    }

    Derived& operator%=(const T rhs) {
        call_scalar_op(*this, rhs, std::modulus<T>());
        return derived();
    }

    template<typename U>
    Mat<T, CHANNELS> operator%(const U &rhs) const {
        return eval() %= rhs;
    }

    template<typename OtherDerived>
    Mat<bool, CHANNELS> operator<(const MatBase<T, CHANNELS, OtherDerived> &rhs) const {
        Mat<bool, CHANNELS> mat(size());
        call_piecewise_ex_op(mat, *this, rhs, std::less<T>());
        return mat;
    }

    template<typename OtherDerived>
    Mat<bool, CHANNELS> operator<=(const MatBase<T, CHANNELS, OtherDerived> &rhs) const {
        Mat<bool, CHANNELS> mat(size());
        call_piecewise_ex_op(mat, *this, rhs, std::less_equal<T>());
        return mat;
    }

    template<typename OtherDerived>
    Mat<bool, CHANNELS> operator>(const MatBase<T, CHANNELS, OtherDerived> &rhs) const {
        Mat<bool, CHANNELS> mat(size());
        call_piecewise_ex_op(mat, *this, rhs, std::greater<T>());
        return mat;
    }

    template<typename OtherDerived>
    Mat<bool, CHANNELS> operator>=(const MatBase<T, CHANNELS, OtherDerived> &rhs) const {
        Mat<bool, CHANNELS> mat(size());
        call_piecewise_ex_op(mat, *this, rhs, std::greater_equal<T>());
        return mat;
    }

    template<typename OtherDerived>
    Mat<bool, CHANNELS> operator==(const MatBase<T, CHANNELS, OtherDerived> &rhs) const {
        Mat<bool, CHANNELS> mat(size());
        call_piecewise_ex_op(mat, *this, rhs, std::equal_to<T>());
        return mat;
    }

    template<typename OtherDerived>
    Mat<bool, CHANNELS> operator!=(const MatBase<T, CHANNELS, OtherDerived> &rhs) const {
        Mat<bool, CHANNELS> mat(size());
        call_piecewise_ex_op(mat, *this, rhs, std::not_equal_to<T>());
        return mat;
    }

    [[nodiscard]] Mat<T, CHANNELS> abs() const {
        Mat<T, CHANNELS> result = eval();
        call_self_piecewise_op(result, [](T a) { return a < 0 ? -a : a; });
        return result;
    }

    [[nodiscard]] inline T min() const {
        return call_reduce(*this, std::numeric_limits<T>::max(), [](T a, T b) { return std::min(a, b); });
    }

    [[nodiscard]] inline T max() const {
        return call_reduce(*this, std::numeric_limits<T>::min(), [](T a, T b) { return std::max(a, b); });
    }

    [[nodiscard]] inline bool any() const {
        return call_reduce(*this, false, [](T a, bool b) { return a || b; });
    }

    [[nodiscard]] inline bool all() const {
        return call_reduce(*this, true, [](T a, bool b) { return a && b; });
    }
};

template<typename T, size_t CHANNELS, typename Derived>
Mat<T, CHANNELS> operator+(T lhs, const MatBase<T, CHANNELS, Derived> &rhs) {
    return rhs + lhs;
}

template<typename T, size_t CHANNELS, typename Derived>
Mat<T, CHANNELS> operator*(T lhs, const MatBase<T, CHANNELS, Derived> &rhs) {
    return rhs * lhs;
}

template<typename T, size_t CHANNELS, typename Derived, typename OtherDerived, typename BinaryOp>
void call_piecewise_op(MatBase<T, CHANNELS, Derived> &lhs, const MatBase<T, CHANNELS, OtherDerived> &rhs, BinaryOp op) {
    if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols())
        throw std::invalid_argument("the matrices should have the same size");

    // since the memory may be not continuous, we cannot simply use one loop to perform addition
    for (size_t r = 0; r < lhs.rows(); r++)
        for (size_t c = 0; c < lhs.cols(); c++)
            for (size_t l = 0; l < CHANNELS; l++)
                lhs(r, c, l) = op(lhs(r, c, l), rhs(r, c, l));
}

template<typename T, typename U, typename V, size_t CHANNELS, typename Derived, typename OtherDerived1, typename OtherDerived2, typename BinaryOp>
void call_piecewise_ex_op(MatBase<T, CHANNELS, Derived> &dst, const MatBase<U, CHANNELS, OtherDerived1> &lhs, const MatBase<V, CHANNELS, OtherDerived2> &rhs, BinaryOp op) {
    if (lhs.rows() != rhs.rows() || lhs.cols() != rhs.cols())
        throw std::invalid_argument("the matrices should have the same size");

    for (size_t r = 0; r < lhs.rows(); r++)
        for (size_t c = 0; c < lhs.cols(); c++)
            for (size_t l = 0; l < CHANNELS; l++)
                dst(r, c, l) = op(lhs(r, c, l), rhs(r, c, l));
}

template<typename T, size_t CHANNELS, typename Derived, typename UnaryOp>
void call_self_piecewise_op(MatBase<T, CHANNELS, Derived> &mat, UnaryOp op) {
    for (size_t r = 0; r < mat.rows(); r++)
        for (size_t c = 0; c < mat.cols(); c++)
            for (size_t l = 0; l < CHANNELS; l++)
                mat(r, c, l) = op(mat(r, c, l));
}

template<typename T, size_t CHANNELS, typename Derived, typename BinaryOp>
void call_scalar_op(MatBase<T, CHANNELS, Derived> &lhs, const T rhs, BinaryOp op) {
    for (size_t r = 0; r < lhs.rows(); r++)
        for (size_t c = 0; c < lhs.cols(); c++)
            for (size_t l = 0; l < CHANNELS; l++)
                lhs(r, c, l) = op(lhs(r, c, l), rhs);
}

template<typename T, typename U, size_t CHANNELS, typename Derived, typename BinaryOp>
U call_reduce(const MatBase<T, CHANNELS, Derived> &mat, U initial, BinaryOp op) {
    U result = initial;
    for (size_t r = 0; r < mat.rows(); r++)
        for (size_t c = 0; c < mat.cols(); c++)
            for (size_t l = 0; l < CHANNELS; l++)
                result = op(result, mat(r, c, l));
    return result;
}

template<typename T, size_t CHANNELS>
class Mat : public MatBase<T, CHANNELS, Mat<T, CHANNELS>> {
    size_t rows_, cols_;
    size_t steps_;
    T* p_data_;

    // we use `std::shared_ptr` to manage memory
    // Note: copying of `std::shared_ptr` is thread-safe
    std::shared_ptr<T> data_holder_;

public:
    Mat(size_t rows_, size_t cols_)
            : rows_(rows_), cols_(cols_), steps_(cols_),
              data_holder_(new T[rows_ * cols_ * CHANNELS]{}, [](T *p) { delete []p; }) {
        p_data_ = data_holder_.get();
    }

    explicit Mat(Size size) : Mat(size.rows, size.cols) {}

    Mat(Mat &mat, Rect roi) : rows_(roi.rows), cols_(roi.cols), steps_(mat.steps_), data_holder_(mat.data_holder_) {
        if (roi.r + roi.rows > mat.rows_ || roi.c + roi.cols > mat.cols_)
            throw std::out_of_range("ROI region out of range");
        p_data_ = &mat(roi.r, roi.c, 0);
    }

    template<size_t M, size_t N>
    explicit Mat(const T(&list)[M][N]) : Mat(M, N) {
        (*this).set(list);
    }

    template<size_t M, size_t N>
    explicit Mat(const T(&list)[M][N][CHANNELS]) : Mat(M, N) {
        (*this).set(list);
    }

    Mat(const Mat&) = default;
    Mat(Mat&&) noexcept = default;
    ~Mat() = default;

    template<typename Derived>
    Mat& operator=(const MatBase<T, CHANNELS, Derived> &rhs) {
        call_piecewise_op(*this, rhs, binary_assignment<T>());
        return *this;
    }

    Mat& operator=(const Mat &rhs) {
        if (this != &rhs)
            call_piecewise_op(*this, rhs, binary_assignment<T>());
        return *this;
    }

    Mat& operator=(Mat&& mat) {
        return *this = mat;
    }

    [[nodiscard]] inline size_t rows() const { return rows_; }
    [[nodiscard]] inline size_t cols() const { return cols_; }
    [[nodiscard]] inline size_t steps() const { return steps_; }
    [[nodiscard]] inline T *data() const { return p_data_; }
    [[nodiscard]] inline Size size() const { return {rows_, cols_}; }

    MatChannelProxy<T, CHANNELS> channel(const size_t channel) {
        if (channel >= CHANNELS)
            throw std::out_of_range("channel out of range");
        return MatChannelProxy(*this, channel);
    }

    MatChannelProxyConst<T, CHANNELS> channel_const(const size_t channel) const {
        if (channel >= CHANNELS)
            throw std::out_of_range("channel out of range");
        return MatChannelProxy(*this, channel);
    }

    MatTransposeProxy<T, CHANNELS> transpose() {
        return MatTransposeProxy(*this);
    }

    // do not check the argument, if needed, use `at` instead
    T operator()(size_t r, size_t c, size_t channel = 0) const {
        return p_data_[ (r * steps_ + c) * CHANNELS + channel ];
    }

    T& operator()(size_t r, size_t c, size_t channel = 0) {
        return p_data_[ (r * steps_ + c) * CHANNELS + channel ];
    }

    [[nodiscard]] Mat block(Rect roi) {
        return Mat(*this, roi);
    }
};

template<typename T, size_t CHANNELS>
class MatChannelProxyConst : public MatBase<T, 1, MatChannelProxy<T, CHANNELS>> {
protected:
    Mat<T, CHANNELS> mat_;
    size_t channel_;

public:
    MatChannelProxyConst(Mat<T, CHANNELS> mat, size_t channel) : mat_(mat), channel_(channel) {}

    [[nodiscard]] inline size_t rows() const { return mat_.rows(); }
    [[nodiscard]] inline size_t cols() const { return mat_.cols(); }
    [[nodiscard]] inline Size size() const { return mat_.size(); }

    // parameter `channel` is ignored
    T operator()(size_t r, size_t c, size_t = 0) const {
        return mat_(r, c, channel_);
    }
};

template<typename T, size_t CHANNELS>
class MatChannelProxy : public MatChannelProxyConst<T, CHANNELS> {
public:
    MatChannelProxy(Mat<T, CHANNELS> mat, size_t channel) : MatChannelProxyConst<T, CHANNELS>(mat, channel) {}

    template<typename OtherDerived>
    MatChannelProxy& operator=(const MatBase<T, 1, OtherDerived> &rhs) {
        call_piecewise_op(*this, rhs, binary_assignment<T>());
        return *this;
    }

    T operator()(size_t r, size_t c, size_t = 0) const {
        return this->mat_(r, c, this->channel_);
    }

    T& operator()(size_t r, size_t c, size_t = 0) {
        return this->mat_(r, c, this->channel_);
    }
};

template<typename T, size_t CHANNELS>
class MatTransposeProxy : public MatBase<T, CHANNELS, MatTransposeProxy<T, CHANNELS>> {
    Mat<T, CHANNELS> mat_;

public:
    explicit MatTransposeProxy(Mat<T, CHANNELS> mat) : mat_(mat) {}

    [[nodiscard]] inline size_t rows() const { return mat_.rows(); }
    [[nodiscard]] inline size_t cols() const { return mat_.cols(); }
    [[nodiscard]] inline Size size() const { return mat_.size(); }

    template<typename OtherDerived>
    MatTransposeProxy& operator=(const MatBase<T, 1, OtherDerived> &rhs) {
        call_piecewise_op(*this, rhs, binary_assignment<T>());
        return *this;
    }

    T operator()(size_t r, size_t c, size_t channel = 0) const {
        return this->mat_(c, r, channel);
    }

    T& operator()(size_t r, size_t c, size_t channel = 0) {
        return this->mat_(c, r, channel);
    }
};

template<typename> inline constexpr bool always_false_v = false;

template<typename T>
void call_openblas_gemm(size_t M, size_t N, size_t K, T alpha, T *A, size_t lda, T *B, size_t ldb, T beta, T *C, size_t ldc) {
    if constexpr (std::is_same_v<T, double>)
        cblas_dgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
    else if constexpr (std::is_same_v<T, float>)
        cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, M, N, K, alpha, A, lda, B, ldb, beta, C, ldc);
    else
        static_assert(always_false_v<T>, "only double and float are supported");
}

template<typename T, size_t CHANNELS, typename Derived, typename OtherDerived>
Mat<T, CHANNELS> operator*(const MatBase<T, CHANNELS, Derived> &lhs, const MatBase<T, CHANNELS, OtherDerived> &rhs) {
    if (lhs.cols() != rhs.rows())
        throw std::invalid_argument("matrices size mismatched in multiplication");
    Mat<T, CHANNELS> result(lhs.rows(), rhs.cols());

    for (size_t l = 0; l < CHANNELS; l++)
        for (size_t r = 0; r < lhs.rows(); r++)
            for (size_t k = 0; k < lhs.cols(); k++)
                for (size_t c = 0; c < rhs.cols(); c++)
                    result(r, c, l) += lhs(r, k, l) * rhs(k, c, l);

    return result;
}

template<typename T, size_t CHANNELS>
Mat<T, CHANNELS> operator*(const Mat<T, CHANNELS> &lhs, const Mat<T, CHANNELS> &rhs) {
    if (lhs.cols() != rhs.rows())
        throw std::invalid_argument("matrices size mismatched in multiplication");
    Mat<T, CHANNELS> result(lhs.rows(), rhs.cols());

    if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
        if constexpr (CHANNELS == 1) {
            call_openblas_gemm<T>(lhs.rows(), lhs.cols(), rhs.cols(), 1., lhs.data(), lhs.steps(), rhs.data(), rhs.steps(), 0., result.data(), result.steps());
        } else {
            Mat<T> A(lhs.size()), B(rhs.size()), C(result.size());
            for (size_t l = 0; l < CHANNELS; l++) {
                A = lhs.channel_const(l);
                B = rhs.channel_const(l);
                call_openblas_gemm<T>(A.rows(), A.cols(), B.cols(), 1., A.data(), A.steps(), B.data(), B.steps(), 0., C.data(), C.steps());
                result.channel(l) = C;
            }
        }
    } else {
        for (size_t l = 0; l < CHANNELS; l++)
            for (size_t r = 0; r < lhs.rows(); r++)
                for (size_t k = 0; k < lhs.cols(); k++)
                    for (size_t c = 0; c < rhs.cols(); c++)
                        result(r, c, l) += lhs(r, k, l) * rhs(k, c, l);
    }

    return result;
}

template<typename T, size_t CHANNELS, typename Derived>
std::ostream &operator<<(std::ostream &stream, const MatBase<T, CHANNELS, Derived> &mat) {
    for (size_t r = 0; r < mat.rows(); r++) {
        stream << " ["[r == 0];

        for (size_t c = 0; c < mat.cols(); c++) {
            if (c)
                stream << ", ";

            if constexpr (CHANNELS == 1)
                stream << mat(r, c);
            else {
                for (size_t l = 0; l < CHANNELS; l++)
                    stream << (l == 0 ? "(" : ", ") << mat(r, c, l);
                stream << ")";
            }
        }

        stream << (r == mat.rows() - 1 ? "]" : ",") << std::endl;
    }

    return stream;
}

#endif // _MATRIX_CLASS_H_
