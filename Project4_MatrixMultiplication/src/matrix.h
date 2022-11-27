#ifndef MATRIX_MULTIPLICATION_MATRIX_H
#define MATRIX_MULTIPLICATION_MATRIX_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _matrix {
    float *p_data;
    size_t rows;
    size_t cols;
    size_t size;
} matrix;

matrix* new_matrix(size_t rows, size_t cols);
void free_matrix(matrix *mat);

void randomize_matrix(matrix *mat);

matrix* matmul_plain(const matrix *lhs, const matrix *rhs);
matrix* matmul_improved(const matrix *lhs, const matrix *rhs);

typedef float __attribute__((aligned(1024))) float_aligned;

void plain_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void plain_krc_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void plain_kcr_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void plain_crk_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void plain_ckr_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void plain_rkc_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void plain_rkc_blocked_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void plain_rkc_blocked_openmp_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);

void gepb_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void gepb_packed_b_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void gepb_packed_a_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void gepb_final_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);
void gepb_aligned_gemm(size_t N, size_t M, size_t K, const float_aligned* __restrict__ lhs, const float_aligned* __restrict__ rhs, float_aligned* __restrict__ dst);
void gepb_parallel_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);

void openblas_gemm(size_t N, size_t M, size_t K, const float *lhs, const float *rhs, float *dst);

#ifdef __cplusplus
}
#endif

#endif  // MATRIX_MULTIPLICATION_MATRIX_H
