#include "matrix.h"

#include <stdlib.h>

matrix* new_matrix(size_t rows, size_t cols) {
    matrix *mat = malloc(sizeof(matrix));
    if (!mat)
        return NULL;

    mat->rows = rows;
    mat->cols = cols;

    size_t size = rows * cols * sizeof(((matrix*)NULL)->p_data[0]);
    mat->size = size;
    mat->p_data = aligned_alloc(1024, size);

    if (!mat->p_data) {
        free(mat);
        return NULL;
    }

    return mat;
}

void free_matrix(matrix *mat) {
    if (mat)
        free(mat->p_data);
    free(mat);
}

void randomize_matrix(matrix *mat) {
    if (!mat || !mat->p_data)
        return;

    size_t elements = mat->rows * mat->cols;
    for (size_t i = 0; i < elements; i++)
        mat->p_data[i] = (float)((double)rand() / RAND_MAX * 10.0);
}

matrix* matmul_plain(const matrix *lhs, const matrix *rhs) {
    if (lhs == NULL || lhs->p_data == NULL || rhs == NULL || rhs->p_data == NULL)
        return NULL;
    if (lhs->cols != rhs->rows)
        return NULL;

    matrix *dst = new_matrix(lhs->rows, rhs->cols);
    if (dst == NULL)
        return NULL;

    plain_gemm(dst->rows, dst->cols, lhs->cols, lhs->p_data, rhs->p_data, dst->p_data);

    return dst;
}

matrix* matmul_improved(const matrix *lhs, const matrix *rhs) {
    if (lhs == NULL || lhs->p_data == NULL || rhs == NULL || rhs->p_data == NULL)
        return NULL;
    if (lhs->cols != rhs->rows)
        return NULL;

    matrix *dst = new_matrix(lhs->rows, rhs->cols);
    if (dst == NULL)
        return NULL;

    gepb_aligned_gemm(dst->rows, dst->cols, lhs->cols, lhs->p_data, rhs->p_data, dst->p_data);

    return dst;
}
