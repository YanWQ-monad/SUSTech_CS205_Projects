#include "matrix.h"

#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef NDEBUG
# define PRINT_WARNING(fmt, ...) fprintf(stderr, fmt "\n", ##__VA_ARGS__)
#else
# define PRINT_WARNING(fmt, ...) ((void)0)
#endif

#define CHECK_ARGUMENT_NOT_NULL(argument)                                         \
    if ((argument) == NULL) {                                                     \
        PRINT_WARNING("%s: got unexpected NULL argument: " #argument, __func__);  \
        return MATRIX_ARGUMENT_INVALID;                                           \
    } 0
//    ^ add a empty zero here, so when we using this macro and add a semicolon after it,
//      clang-tiny won't warn "empty statement"

#define CHECK_MATRIX_NOT_NULL(matrix)                                                  \
    if ((matrix) == NULL || (matrix)->p_data == NULL) {                                \
        PRINT_WARNING("%s: got unexpected NULL matrix argument: " #matrix, __func__);  \
        return MATRIX_NULL_MATRIX;                                                     \
    } 0

#define TRY(expression) {            \
        int code = (expression);     \
        if (code != MATRIX_SUCCESS)  \
            return code;             \
    } 0

typedef float (*binaryOpFunc)(float, float);

static inline size_t calculateMatrixSize(const size_t rows, const size_t cols) {
    // `sizeof` is compile-time operator and the value only depends on the type,
    // so dereferencing an invalid pointer is safe here (since it won't be evaluated).
    size_t unit_size = sizeof(((matrix*)NULL)->p_data[0]);

    size_t size = rows * cols * unit_size;
    return size;
}

// set `rows` and `cols`, and allocate corresponding memory, but do NOT initialize the data
//
// WARNING: the data of `p_data` is uninitialized, so make sure you initialize it before you use it
static int rawCreateMatrix(matrix *mat, const size_t rows, const size_t cols) {
    CHECK_ARGUMENT_NOT_NULL(mat);

    mat->rows = rows;
    mat->cols = cols;

    size_t size = calculateMatrixSize(rows, cols);
    mat->p_data = malloc(size);
    if (mat->p_data == NULL) {
        PRINT_WARNING("%s: failed to allocate %zu bytes for the matrix\n", __func__, size);
        return MATRIX_ALLOC_FAILED;
    }

    return MATRIX_SUCCESS;
}

int createMatrix(matrix *mat, const Size size) {
    CHECK_ARGUMENT_NOT_NULL(mat);

    TRY(rawCreateMatrix(mat, size.rows, size.cols));

    size_t capacity = calculateMatrixSize(size.rows, size.cols);
    memset(mat->p_data, 0, capacity);

    return MATRIX_SUCCESS;
}

matrix* createMatrixEx(const Size size) {
    matrix *mat = malloc(sizeof(matrix));
    if (mat == NULL) {
        PRINT_WARNING("%s: failed to allocate %zu bytes for the matrix\n", __func__, sizeof(matrix));
        return NULL;
    }

    int r = createMatrix(mat, size);
    if (r != MATRIX_SUCCESS) {
        free(mat);
        return NULL;
    }

    return mat;
}

matrix* createUnallocatedMatrix(void) {
    matrix *mat = malloc(sizeof(matrix));
    memset(mat, 0, sizeof(matrix));
    return mat;
}

void deleteMatrix(matrix *mat) {
    if (mat == NULL)
        return;

    // If `mat` is already freed, it's safe to free it again
    // since `free(NULL)` will not perform any operation.
    free(mat->p_data);

    mat->rows = 0;
    mat->cols = 0;
    mat->p_data = NULL;
}

void deleteMatrixEx(matrix *mat) {
    // The two functions are both NULL-safe
    deleteMatrix(mat);
    free(mat);
}

int saveMatrix(const matrix *mat, const char *path) {
    CHECK_MATRIX_NOT_NULL(mat);
    CHECK_ARGUMENT_NOT_NULL(path);

    FILE *f = fopen(path, "wb");
    if (f == NULL)
        return MATRIX_IO_FAILED;

    if (!fwrite(&mat->rows, sizeof(mat->rows), 1, f) ||
        !fwrite(&mat->cols, sizeof(mat->cols), 1, f) ||
        !fwrite(mat->p_data, sizeof(mat->p_data[0]), mat->rows * mat->cols, f))
    {
        fclose(f);
        return MATRIX_IO_FAILED;
    }

    fclose(f);
    return MATRIX_SUCCESS;
}

int loadMatrix(matrix *mat, const char *path) {
    CHECK_ARGUMENT_NOT_NULL(mat);
    CHECK_ARGUMENT_NOT_NULL(path);

    FILE *f = fopen(path, "rb");
    if (f == NULL)
        return MATRIX_IO_FAILED;

    size_t rows, cols;
    if (!fread(&rows, sizeof(rows), 1, f) || !fread(&cols, sizeof(cols), 1, f)) {
        fclose(f);
        return MATRIX_IO_FAILED;
    }

    int r = rawCreateMatrix(mat, rows, cols);
    if (r != MATRIX_SUCCESS) {
        fclose(f);
        return r;
    }

    if (!fread(mat->p_data, sizeof(mat->p_data[0]), rows * cols, f)) {
        fclose(f);
        deleteMatrix(mat);
        return MATRIX_IO_FAILED;
    }

    fclose(f);
    return MATRIX_SUCCESS;
}

int loadMatrixFromArray(matrix *dst, const float *src) {
    CHECK_MATRIX_NOT_NULL(dst);
    CHECK_ARGUMENT_NOT_NULL(src);

    size_t size = calculateMatrixSize(dst->rows, dst->cols);
    memcpy(dst->p_data, src, size);

    return MATRIX_SUCCESS;
}

bool matrixEqual(const matrix *lhs, const matrix *rhs, const float eps) {
    if (lhs == NULL || lhs->p_data == NULL || rhs == NULL || rhs->p_data == NULL)
        return false;
    if (lhs->rows != rhs->rows || lhs->cols != rhs->cols)
        return false;
    size_t size = lhs->rows * lhs->cols;
    for (size_t i = 0; i < size; i++)
        if (fabsf(lhs->p_data[i] - rhs->p_data[i]) > eps)
            return false;
    return true;
}

int cutMatrix(matrix *dst, const matrix *src, const Position start, const Size size) {
    CHECK_ARGUMENT_NOT_NULL(dst);
    CHECK_MATRIX_NOT_NULL(src);

    if (start.r > src->rows || start.c > src->cols) {
        PRINT_WARNING("%s: the top left corner (%zu, %zu) is out of range, the matrix size is (%zu, %zu)",
                      __func__, start.r, start.c, src->rows, src->cols);
        return MATRIX_OUT_OF_RANGE;
    }
    if (start.r + size.rows > src->rows || start.c + size.cols > src->cols) {
        PRINT_WARNING("%s: the bottom right corner (%zu, %zu) is out of range, the matrix size is (%zu, %zu)",
                      __func__, start.r + size.rows, start.c + size.cols, src->rows, src->cols);
        return MATRIX_OUT_OF_RANGE;
    }

    TRY(rawCreateMatrix(dst, size.rows, size.cols));

    for (size_t r = 0; r < size.rows; r++)
        for (size_t c = 0; c < size.cols; c++)
            setElementAt(dst, (Position){r, c}, getElementAt(src, (Position){r + start.r, c + start.c}));

    return MATRIX_SUCCESS;
}

int cutMatrix2(matrix *dst, const matrix *src, Position start, Position end) {
    if (end.r < start.r || end.c < start.c) {
        PRINT_WARNING("%s: the end corner (%zu, %zu) should be at the bottom right side of start corner (%zu, %zu)",
                      __func__, end.r, end.c, start.r, start.c);
        return MATRIX_OUT_OF_RANGE;
    }

    Size size = { end.r - start.r + 1, end.c - start.c + 1 };
    return cutMatrix(dst, src, start, size);
}

int joinMatrixAlongRows(matrix *dst, const matrix *upper, const matrix *lower) {
    CHECK_ARGUMENT_NOT_NULL(dst);
    CHECK_MATRIX_NOT_NULL(upper);
    CHECK_MATRIX_NOT_NULL(lower);

    if (upper->cols != lower->cols) {
        PRINT_WARNING("%s: the matrices with size (%zu, %zu) and (%zu, %zu) have different columns, cannot join",
                      __func__, upper->rows, upper->cols, lower->rows, lower->cols);
        return MATRIX_SIZE_MISMATCHED;
    }

    TRY(rawCreateMatrix(dst, upper->rows + lower->rows, upper->cols));

    memcpy(dst->p_data, upper->p_data, calculateMatrixSize(upper->rows, upper->cols));
    size_t offset = upper->rows * upper->cols;
    memcpy(dst->p_data + offset, lower->p_data, calculateMatrixSize(lower->rows, lower->cols));

    return MATRIX_SUCCESS;
}

int joinMatrixAlongColumns(matrix *dst, const matrix *lhs, const matrix *rhs) {
    CHECK_ARGUMENT_NOT_NULL(dst);
    CHECK_MATRIX_NOT_NULL(lhs);
    CHECK_MATRIX_NOT_NULL(rhs);

    if (lhs->rows != rhs->rows) {
        PRINT_WARNING("%s: the matrices with size (%zu, %zu) and (%zu, %zu) have different rows, cannot join",
                      __func__, lhs->rows, lhs->cols, rhs->rows, rhs->cols);
        return MATRIX_SIZE_MISMATCHED;
    }

    TRY(rawCreateMatrix(dst, lhs->rows, lhs->cols + rhs->cols));

    for (size_t r = 0; r < dst->rows; r++) {
        for (size_t c = 0; c < lhs->cols; c++)
            setElementAt(dst, (Position){r, c}, getElementAt(lhs, (Position){r, c}));
        for (size_t c = 0; c < rhs->cols; c++)
            setElementAt(dst, (Position){r, c + lhs->cols}, getElementAt(rhs, (Position){r, c}));
    }

    return MATRIX_SUCCESS;
}

int fillMatrix(matrix *mat, float value) {
    CHECK_MATRIX_NOT_NULL(mat);

    size_t size = mat->rows * mat->cols;
    for (size_t i = 0; i < size; i++)
        mat->p_data[i] = value;

    return MATRIX_SUCCESS;
}

int fillMatrixWith(matrix *mat, float (*func)(size_t x, size_t y)) {
    CHECK_MATRIX_NOT_NULL(mat);
    CHECK_ARGUMENT_NOT_NULL(func);

    for (size_t r = 0; r < mat->rows; r++)
        for (size_t c = 0; c < mat->cols; c++)
            setElementAt(mat, (Position){r, c}, func(r, c));

    return MATRIX_SUCCESS;
}

int copyMatrix(matrix *dst, const matrix *src) {
    CHECK_ARGUMENT_NOT_NULL(dst);
    CHECK_MATRIX_NOT_NULL(src);

    TRY(rawCreateMatrix(dst, src->rows, src->cols));

    // guarantee to be no error, since `dst` and `src->p_data` are all valid
    loadMatrixFromArray(dst, src->p_data);

    return MATRIX_SUCCESS;
}

static int arithmOp(matrix *dst, const matrix *lhs, const matrix *rhs, const binaryOpFunc func) {
    CHECK_ARGUMENT_NOT_NULL(dst);
    CHECK_MATRIX_NOT_NULL(lhs);
    CHECK_MATRIX_NOT_NULL(rhs);

    if (lhs->rows != rhs->rows || lhs->cols != rhs->cols) {
        PRINT_WARNING("%s: matrices size (%zu, %zu) and (%zu, %zu) are mismatched",
                      __func__, lhs->rows, lhs->cols, rhs->rows, rhs->cols);
        return MATRIX_SIZE_MISMATCHED;
    }

    TRY(rawCreateMatrix(dst, lhs->rows, lhs->cols));

    size_t size = lhs->rows * lhs->cols;
    for (size_t i = 0; i < size; i++)
        dst->p_data[i] = func(lhs->p_data[i], rhs->p_data[i]);

    return MATRIX_SUCCESS;
}

static int scaleOp(matrix *dst, const matrix *lhs, const float rhs, const binaryOpFunc func) {
    CHECK_ARGUMENT_NOT_NULL(dst);
    CHECK_MATRIX_NOT_NULL(lhs);

    TRY(rawCreateMatrix(dst, lhs->rows, lhs->cols));

    size_t size = lhs->rows * lhs->cols;
    for (size_t i = 0; i < size; i++)
        dst->p_data[i] = func(lhs->p_data[i], rhs);

    return MATRIX_SUCCESS;
}

static float binaryAdd(const float lhs, const float rhs) { return lhs + rhs; }
static float binarySub(const float lhs, const float rhs) { return lhs - rhs; }
static float binaryMul(const float lhs, const float rhs) { return lhs * rhs; }
static float binaryDiv(const float lhs, const float rhs) { return lhs / rhs; }
static float binaryMax(const float lhs, const float rhs) { return lhs > rhs ? lhs : rhs; }
static float binaryMin(const float lhs, const float rhs) { return lhs < rhs ? lhs : rhs; }

int addMatrix(matrix *dst, const matrix *lhs, const matrix *rhs) {
    return arithmOp(dst, lhs, rhs, binaryAdd);
}

int subMatrix(matrix *dst, const matrix *lhs, const matrix *rhs) {
    return arithmOp(dst, lhs, rhs, binarySub);
}

int addMatrixScale(matrix *dst, const matrix *lhs, const float rhs) {
    return scaleOp(dst, lhs, rhs, binaryAdd);
}

int subMatrixScale(matrix *dst, const matrix *lhs, const float rhs) {
    return scaleOp(dst, lhs, rhs, binarySub);
}

int mulMatrixScale(matrix *dst, const matrix *lhs, const float rhs) {
    return scaleOp(dst, lhs, rhs, binaryMul);
}

int divMatrixScale(matrix *dst, const matrix *lhs, const float rhs) {
    return scaleOp(dst, lhs, rhs, binaryDiv);
}

int mulMatrix(matrix *dst, const matrix *lhs, const matrix *rhs) {
    CHECK_ARGUMENT_NOT_NULL(dst);
    CHECK_MATRIX_NOT_NULL(lhs);
    CHECK_MATRIX_NOT_NULL(rhs);

    if (lhs->cols != rhs->rows) {
        PRINT_WARNING("%s: matrices with size (%zu, %zu) and (%zu, %zu) cannot be multiplied",
                      __func__, lhs->rows, lhs->cols, rhs->rows, rhs->cols);
        return MATRIX_SIZE_MISMATCHED;
    }

    TRY(rawCreateMatrix(dst, lhs->rows, rhs->cols));

    for (size_t r = 0; r < lhs->rows; r++)
        for (size_t c = 0; c < rhs->cols; c++) {
            float sum = 0;
            for (size_t k = 0; k < lhs->cols; k++)
                sum += lhs->p_data[indexAt(r, k, lhs->cols)] * rhs->p_data[indexAt(k, c, rhs->cols)];
            dst->p_data[indexAt(r, c, dst->cols)] = sum;
        }

    return MATRIX_SUCCESS;
}

static float reduceInMatrix(const matrix *mat, int *error_code, const binaryOpFunc reducer, const float initial) {
    if (mat == NULL || mat->p_data == NULL) {
        PRINT_WARNING("%s: got unexpected NULL matrix argument: mat", __func__);
        if (error_code)
            *error_code = MATRIX_NULL_MATRIX;
        return NAN;
    }

    float value = initial;
    size_t size = mat->rows * mat->cols;
    for (size_t i = 0; i < size; i++)
        value = reducer(value, mat->p_data[i]);

    if (error_code)
        *error_code = MATRIX_SUCCESS;

    return value;
}

float minInMatrix(const matrix *mat, int *error_code) {
    return reduceInMatrix(mat, error_code, binaryMin, FLT_MAX);
}

float maxInMatrix(const matrix *mat, int *error_code) {
    return reduceInMatrix(mat, error_code, binaryMax, -FLT_MAX);
}

// printf only accept double as floating number.
// If we pass a float to it, it will automatically be converted to double, which is unavoidable
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdouble-promotion"
static void printMatrixRow(const matrix *mat, const size_t r) {
    size_t cols = mat->cols;
    if (cols <= 8) {
        printf("%12g", mat->p_data[indexAt(r, 0, cols)]);
        for (size_t c = 1; c < cols; c++)
            printf(", %12g", mat->p_data[indexAt(r, c, cols)]);
    }
    else {
        printf("%12g, %12g, %12g,   ...  , %12g, %12g, %12g",
               mat->p_data[indexAt(r, 0, cols)],
               mat->p_data[indexAt(r, 1, cols)],
               mat->p_data[indexAt(r, 2, cols)],
               mat->p_data[indexAt(r, cols - 3, cols)],
               mat->p_data[indexAt(r, cols - 2, cols)],
               mat->p_data[indexAt(r, cols - 1, cols)]);
    }
}

int printMatrix(const matrix *mat) {
    CHECK_MATRIX_NOT_NULL(mat);

    for (size_t r = 0; r < mat->rows; r++) {
        printf(r == 0 ? "[[ " : " [ ");

        if (r == 3 && mat->rows > 8) {
            if (mat->cols > 8)
                printf("     ...    ,      ...    ,      ...    ,   ...  ,      ...    ,      ...    ,      ...    ");
            else {
                printf("     ...    ");
                for (size_t c = 1; c < mat->cols; c++)
                    printf(",      ...    ");
            }
            r = mat->rows - 4;
        }
        else {
            printMatrixRow(mat, r);
        }

        puts(r == mat->rows - 1 ? "]]" : "],");
    }

    return MATRIX_SUCCESS;
}
#pragma GCC diagnostic pop

// since the series functions `fill_*` need to share a same signature,
// so it's natural that some parameters are unused in some fillings
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
float fill_identity(size_t x, size_t y) {
    return x == y ? 1.f : 0.f;
}

float fill_ones(size_t x, size_t y) {
    return 1.f;
}

float fill_zeros(size_t x, size_t y) {
    return 0.f;
}
#pragma GCC diagnostic pop
