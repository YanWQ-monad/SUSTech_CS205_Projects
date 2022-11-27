#ifndef MATRIX_LIBRARY_MATRIX_H_
#define MATRIX_LIBRARY_MATRIX_H_

//! @file matrix.h

// If a C++ file include this header, we need "extern C" to make linking work properly
#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <math.h>

//! @name Error codes
//!@{

//! operation success, the value is guaranteed to be `0`
#define MATRIX_SUCCESS 0

//! argument invalid, such as pass NULL as a pointer
#define MATRIX_ARGUMENT_INVALID 1

//! the matrix is unallocated, please use `createMatrix` or so on to initialize it first
#define MATRIX_NULL_MATRIX 2

//! memory allocation failed. `errno` can be used to determinate the detailed error
#define MATRIX_ALLOC_FAILED 3

//! matrix size mismatched
#define MATRIX_SIZE_MISMATCHED 4

//! area or coordinate out of range
#define MATRIX_OUT_OF_RANGE 5

//! IO error when saving or loading
#define MATRIX_IO_FAILED 6
//!@}

//! @brief The matrix struct
typedef struct matrix {
    //! The rows of matrix
    size_t rows;

    //! The columns of matrix
    size_t cols;

    //! The pointer to the data. If it's NULL, the matrix is unallocated.
    float *p_data;
} matrix;

typedef struct Position {
    size_t r;
    size_t c;
} Position;

typedef struct Size {
    size_t rows;
    size_t cols;
} Size;

//! @name Construction and destruction
//!@{

//! @brief Create a matrix with size (rows, cols).
//!
//! It will allocate corresponding memory, and set all the elements to zero.
//!
//! @code
//!   matrix mat;
//!   createMatrix(&mat, 2, 2);
//!   // ... do work ...
//!   deleteMatrix(&mat);
//! @endcode
//!
//! @sa createMatrixEx, deleteMatrix
//!
//! @param [out] mat The address of matrix that needs to be initialized
//! @param [in] size The size (rows and columns) of the matrix
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ARGUMENT_INVALID: `mat` is NULL
//! @returns MATRIX_ALLOC_FAILED: `malloc` failed
int createMatrix(matrix *mat, Size size);

//! @brief Create a matrix with size (rows, cols).
//!
//! It will allocate corresponding memory, and set all the elements to zero.
//!
//! @code
//!   matrix *mat = createMatrixEx((Size){2, 2});
//!   // ... do work ...
//!   deleteMatrixEx(mat);
//! @endcode
//!
//! @sa createMatrix, deleteMatrixEx
//!
//! @param [in] size The size (rows and columns) of the matrix
//!
//! @returns NULL if failed to allocate, otherwise is allocated matrix
matrix *createMatrixEx(Size size);

//! @brief Create an unallocated matrix.
//!
//! It will allocated the pointer itself, but the data is NULL.
//! Can be used in accepting operation results.
//!
//! @sa createMatrix, createMatrixEx
//!
//! @returns NULL if failed to malloc
matrix *createUnallocatedMatrix(void);

//! @brief Release the memory that the matrix owns, and set `p_data` to NULL.
//!
//! If `mat->p_data` is already NULL, no operation is performed.
//! @sa createMatrix
//!
//! @param [in] mat The address of matrix to release
void deleteMatrix(matrix *mat);

//! @brief Release the matrix data and matrix itself.
//!
//! If `mat` is already NULL, no operation is performed.
//! @sa createMatrixEx
//!
//! @param [in] mat The address of matrix to release
void deleteMatrixEx(matrix *mat);

//! @brief Save matrix to file.
//! @sa loadMatrix
//!
//! @param [in] mat The matrix to save
//! @param [in] path The file path
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ARGUMENT_INVALID: `mat` is NULL or unallocated
//! @returns MATRIX_IO_FAILED: IO failed when saving
int saveMatrix(const matrix *mat, const char *path);

//! @brief Load matrix from file.
//! @sa saveMatrix
//!
//! @param [in] mat The matrix to load (unallocated)
//! @param [in] path The file path
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ARGUMENT_INVALID: `mat` is NULL
//! @returns MATRIX_IO_FAILED: IO failed when saving
//! @returns MATRIX_ALLOC_FAILED: allocation failed
int loadMatrix(matrix *mat, const char *path);

//!@}

//! @name Getter and setter
//!@{

//! @brief Mapping 2D coordinates to 1D index in the matrix
//! @sa getElementAt
//!
//! @param [in] r The row number
//! @param [in] c The column number
//! @param [in] cols The columns in the matrix
//!
//! @returns the 1D index in matrix
static inline size_t indexAt(const size_t r, const size_t c, const size_t cols) {
    return r * cols + c;
}

//! @brief Get value of position (r, c) of the matrix.
//! @sa indexAt, setElementAt
//!
//! @param [in] mat The matrix (allocated)
//! @param [in] pos The position marked by (rows, columns)
//!
//! @returns the corresponding value.
//! If matrix is NULL or unallocated, or the position is out of range, NAN is returned.
static inline float getElementAt(const matrix *mat, const Position pos) {
    if (mat == NULL || mat->p_data == NULL || pos.r >= mat->rows || pos.c >= mat->cols)
        return NAN;
    return mat->p_data[indexAt(pos.r, pos.c, mat->cols)];
}

//! @brief Set value of position (r, c) of the matrix.
//! @sa getElementAt
//!
//! @param [in,out] mat The matrix (allocated)
//! @param [in] pos The position marked by (rows, columns)
//! @param [in] value The value
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_NULL_MATRIX: `mat` is NULL or unallocated
//! @returns MATRIX_OUT_OF_RANGE: (r, c) is out of range
static inline int setElementAt(matrix *mat, const Position pos, const float value) {
    if (mat == NULL || mat->p_data == NULL)
        return MATRIX_NULL_MATRIX;
    if (pos.r >= mat->rows || pos.c >= mat->cols)
        return MATRIX_OUT_OF_RANGE;
    mat->p_data[indexAt(pos.r, pos.c, mat->cols)] = value;
    return MATRIX_SUCCESS;
}

//! @brief Print the overall (not all) data of the matrix.
//!
//! @param [in] mat The matrix to print.
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns  MATRIX_NULL_MATRIX: `src` is NULL
int printMatrix(const matrix *mat);

//!@}

//! @name Comparators
//!@{

//! @brief Compare whether the matrices are equal under given EPS
//!
//! @param lhs The matrix
//! @param rhs The matrix
//! @param eps The tolerable error
//!
//! @returns If any matrix is NULL, or they have different sizes, `false` is returned.
//! Otherwise returns whether all the elements are equal
bool matrixEqual(const matrix *lhs, const matrix *rhs, float eps);

//!@}

//! @name Basic matrix operations
//!@{

//! @brief Copy a matrix `src` to `dst`.
//!
//! If `dst` has data, the function won't help you to free it.
//!
//! @param [out] dst The matrix to store the copied data (unallocated)
//! @param [in] src The original matrix to be copied
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ALLOC_FAILED: `malloc` failed
//! @returns MATRIX_ARGUMENT_INVALID: `dst` is NULL
//! @returns MATRIX_NULL_MATRIX: `src` is NULL
int copyMatrix(matrix *dst, const matrix *src);

//! @brief Load data to `dst` from primitive array `src`.
//!
//! It will copy `rows` * `cols` elements from `src` to `dst`.
//!
//! @param [in,out] dst The target matrix (allocated)
//! @param [in] src The data source
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ARGUMENT_INVALID: `dst` is NULL
//! @returns MATRIX_NULL_MATRIX: `src` is NULL
int loadMatrixFromArray(matrix *dst, const float *src);

//! @brief Cut a sub-matrix from a given matrix with given start and size.
//!
//! Consider a matrix
//!
//! \f$
//! \begin{bmatrix}
//!   1 & 2 & 3 & 4 \\
//!   5 & 6 & 7 & 8
//! \end{bmatrix}
//! \f$
//!
//!     cutMatrix(dst, src, 0, 1, 2, 1);
//!
//! will create a new matrix
//!
//! \f$
//! \begin{bmatrix}
//!   2 \\
//!   6
//! \end{bmatrix}
//! \f$
//!
//! @sa cutMatrix2
//!
//! @param [out] dst The cut matrix (unallocated)
//! @param [in] src The original matrix
//! @param [in] start The left top corner of the sub-matrix
//! @param [in] size The size of the sub-matrix
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ARGUMENT_INVALID: `dst` is NULL
//! @returns MATRIX_NULL_MATRIX: `src` is NULL or unallocated
//! @returns MATRIX_OUT_OF_RANGE: the given range is (partially or completely) out-of-range
//! @returns MATRIX_ALLOC_FAILED: allocation failed
int cutMatrix(matrix *dst, const matrix *src, Position start, Size size);

//! @brief Cut a sub-matrix from a given matrix with given two corners.
//!
//! Consider a matrix
//!
//! \f$
//! \begin{bmatrix}
//!   1 & 2 & 3 & 4 \\
//!   5 & 6 & 7 & 8
//! \end{bmatrix}
//! \f$
//!
//!     cutMatrix2(dst, src, 0, 1, 1, 1);
//!
//! will create a new matrix
//!
//! \f$
//! \begin{bmatrix}
//!   2 \\
//!   6
//! \end{bmatrix}
//! \f$
//!
//! @sa cutMatrix
//!
//! @param [out] dst The cut matrix (unallocated)
//! @param [in] src The original matrix
//! @param [in] start The left top corner of the sub-matrix
//! @param [in] end The right bottom corner of the sub-matrix
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ARGUMENT_INVALID: `dst` is NULL
//! @returns MATRIX_NULL_MATRIX: `src` is NULL or unallocated
//! @returns MATRIX_OUT_OF_RANGE: the given range is (partially or completely) out-of-range
//! @returns MATRIX_ALLOC_FAILED: allocation failed
int cutMatrix2(matrix *dst, const matrix *src, Position start, Position end);

//! @brief Join two matrices along rows
//!
//! \f$
//! \operatorname{join} \bigg(
//! \begin{bmatrix}
//!   1 & 2 \\
//!   3 & 4
//! \end{bmatrix}
//! ,\,
//! \begin{bmatrix}
//!   5 & 6 \\
//!   7 & 8
//! \end{bmatrix}
//! \bigg)
//! \rightarrow
//! \begin{bmatrix}
//!   1 & 2 \\
//!   3 & 4 \\
//!   5 & 6 \\
//!   7 & 8
//! \end{bmatrix}
//! \f$
//!
//! @sa joinMatrixAlongColumns
//!
//! @param [out] dst The output (unallocated)
//! @param [in] upper The upper matrix
//! @param [in] lower The lower matrix
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ARGUMENT_INVALID: `dst` is NULL
//! @returns MATRIX_NULL_MATRIX: `upper` or `lower` are NULL or unallocated
//! @returns MATRIX_SIZE_MISMATCHED: `upper` and `lower` have different columns
//! @returns MATRIX_ALLOC_FAILED: allocation failed
int joinMatrixAlongRows(matrix *dst, const matrix *upper, const matrix *lower);

//! @brief Join two matrices along columns
//!
//! \f$
//! \operatorname{join} \bigg(
//! \begin{bmatrix}
//!   1 & 2 \\
//!   3 & 4
//! \end{bmatrix}
//! ,\,
//! \begin{bmatrix}
//!   5 & 6 \\
//!   7 & 8
//! \end{bmatrix}
//! \bigg)
//! \rightarrow
//! \begin{bmatrix}
//!   1 & 2 & 5 & 6 \\
//!   3 & 4 & 7 & 8
//! \end{bmatrix}
//! \f$
//!
//! @sa joinMatrixAlongRows
//!
//! @param [out] dst The output (unallocated)
//! @param [in] lhs The left matrix
//! @param [in] rhs The right matrix
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ARGUMENT_INVALID: `dst` is NULL
//! @returns MATRIX_NULL_MATRIX: `lhs` or `rhs` are NULL or unallocated
//! @returns MATRIX_SIZE_MISMATCHED: `lhs` and `rhs` have different rows
//! @returns MATRIX_ALLOC_FAILED: allocation failed
int joinMatrixAlongColumns(matrix *dst, const matrix *lhs, const matrix *rhs);

//! @brief Fill the matrix with value
//!
//! @sa fillMatrixWith
//!
//! @param [in,out] mat The matrix to fill
//! @param [in] value The value to fill the matrix
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_NULL_MATRIX: `mat` is NULL or unallocated
int fillMatrix(matrix *mat, float value);

//! @brief Fill the matrix using a generator
//!
//! @code
//!   float identity(size_t x, size_t y) {
//!       return x == y ? 1.f : 0.f;
//!   }
//!
//!   matrix *mat = createMatrixEx(3, 3);
//!   fillMatrixWith(mat, identity);
//! @endcode
//!
//! @sa fillMatrix
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_NULL_MATRIX: `mat` is NULL or unallocated
//! @returns MATRIX_ARGUMENT_INVALID: `func` is NULL
int fillMatrixWith(matrix *mat, float (*func)(size_t x, size_t y));

//! @name Preset matrices: generate some special matrices
//!@{

//! Generate an identity matrix
float fill_identity(size_t x, size_t y);

//! Generate a matrix with all ones
float fill_ones(__attribute__((unused)) size_t x, size_t y);

//! Generate a matrix with all zeros
float fill_zeros(size_t x, size_t y);

//!@}

//!@}

//! @name Advanced operations
//!@{

//! @brief Matrix multiplication.
//!
//! @param [out] dst The matrix to store data (unallocated)
//! @param [in] lhs The left operand
//! @param [in] rhs The right operand
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ALLOC_FAILED: failed to allocate for `dst`
//! @returns MATRIX_ARGUMENT_INVALID: `dst` is NULL
//! @returns MATRIX_NULL_MATRIX: `lhs` or `rhs` is NULL matrix
//! @returns MATRIX_SIZE_MISMATCHED: `lhs` and `rhs` can not perform multiplication
int mulMatrix(matrix *dst, const matrix *lhs, const matrix *rhs);

//!@}

//! @name Point-wise (i.e. element-wise) operators
//!
//! \f$ \displaystyle
//!  \begin{bmatrix}
//!  1 & 0 \\
//!  1 & 7
//!  \end{bmatrix}
//!  +
//!  \begin{bmatrix}
//!  1 & 3 \\
//!  -1 & 4
//!  \end{bmatrix}
//!  =
//!  \begin{bmatrix}
//!  2 & 3 \\
//!  0 & 11
//!  \end{bmatrix}
//! \f$
//!
//! @param [out] dst The matrix to store data, should be unallocated
//! @param [in] lhs The left operand
//! @param [in] rhs The right operand
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ALLOC_FAILED: failed to allocate for `dst`
//! @returns MATRIX_ARGUMENT_INVALID: `dst` is NULL
//! @returns MATRIX_NULL_MATRIX: `lhs` or `rhs` is NULL matrix
//! @returns MATRIX_SIZE_MISMATCHED: `lhs` and `rhs` have different sizes
//!@{

//! point-wise addition
int addMatrix(matrix *dst, const matrix *lhs, const matrix *rhs);

//! point-wise subtract
int subMatrix(matrix *dst, const matrix *lhs, const matrix *rhs);

//!@}

//! @name Scale operations
//!
//! \f$ \displaystyle
//!  \begin{bmatrix}
//!  1 & 0 \\
//!  1 & 7
//!  \end{bmatrix}
//!  \times 3
//!  =
//!  \begin{bmatrix}
//!  3 & 0 \\
//!  3 & 21
//!  \end{bmatrix}
//! \f$
//!
//! @param [out] dst The matrix to store data (unallocated)
//! @param [in] lhs The left operand
//! @param [in] rhs The right operand
//!
//! @returns MATRIX_SUCCESS: OK
//! @returns MATRIX_ALLOC_FAILED: failed to allocate for `dst`
//! @returns MATRIX_ARGUMENT_INVALID: `dst` is NULL
//! @returns MATRIX_NULL_MATRIX: `lhs` is NULL matrix
//!@{

//! Addition
int addMatrixScale(matrix *dst, const matrix *lhs, float rhs);

//! Subtract
int subMatrixScale(matrix *dst, const matrix *lhs, float rhs);

//! Multiplication
int mulMatrixScale(matrix *dst, const matrix *lhs, float rhs);

//! Division
int divMatrixScale(matrix *dst, const matrix *lhs, float rhs);

//!@}

//! @name Reduce operations
//!
//! Returns the reduced value, like the maximum or minimum value
//!
//! @param [in] mat The matrix to reduce
//! @param [out] error_code pointer to error code. If it's NULL, no error code is updated
//!
//! @returns The reduced value
//!@{

//! The minimum value in the matrix
float minInMatrix(const matrix *mat, int *error_code);

//! The maximum value in the matrix
float maxInMatrix(const matrix *mat, int *error_code);

//!@}

#ifdef __cplusplus
}
#endif

#endif  // MATRIX_LIBRARY_MATRIX_H_
