#include "matrix.h"

#include <cblas.h>
#include <pthread.h>
#include <stdlib.h>

#define TASK 2

#define CHECK_ARGUMENT \
    if (lhs == NULL || rhs == NULL || dst == NULL) \
        return; 0

#define A(i, j) lhs[ i * M + j ]
#define B(i, j) rhs[ i * K + j ]
#define C(i, j) dst[ i * M + j ]
#define D(i, j) temp[ j * 16 + i ]

void plain_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    for (size_t r = 0; r < N; r++)
        for (size_t c = 0; c < M; c++)
            for (size_t k = 0; k < K; k++)
                C(r, c) += A(r, k) * B(k, c);
}

void plain_rkc_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    for (size_t r = 0; r < N; r++)
        for (size_t k = 0; k < K; k++)
            for (size_t c = 0; c < M; c++)
                C(r, c) += A(r, k) * B(k, c);
}

void plain_krc_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    for (size_t k = 0; k < K; k++)
        for (size_t r = 0; r < N; r++)
            for (size_t c = 0; c < M; c++)
                C(r, c) += A(r, k) * B(k, c);
}

void plain_kcr_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    for (size_t k = 0; k < K; k++)
        for (size_t c = 0; c < M; c++)
            for (size_t r = 0; r < N; r++)
                C(r, c) += A(r, k) * B(k, c);
}

void plain_crk_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    for (size_t c = 0; c < M; c++)
        for (size_t r = 0; r < N; r++)
            for (size_t k = 0; k < K; k++)
                C(r, c) += A(r, k) * B(k, c);
}

void plain_ckr_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    for (size_t c = 0; c < M; c++)
        for (size_t k = 0; k < K; k++)
            for (size_t r = 0; r < N; r++)
                C(r, c) += A(r, k) * B(k, c);
}

void plain_rkc_blocked_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    for (size_t r0 = 0; r0 < N; r0 += 16)
        for (size_t k0 = 0; k0 < K; k0 += 16)
            for (size_t r = 0; r < 16; r++)
                for (size_t k = 0; k < 16; k++)
                    for (size_t c = 0; c < M; c++)
                        C(r0 + r, c) += A(r0 + r, k0 + k) * B(k0 + k, c);
}

// float *A_packed, *C_packed;
//__attribute__((aligned(1024))) float *A_packed, *C_packed;

void gepb_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    for (size_t k0 = 0; k0 < K; k0 += 16)
        for (size_t c0 = 0; c0 < M; c0 += 16)
            for (size_t r = 0; r < N; r++)
                for (size_t c = 0; c < 16; c++)
                    for (size_t k = 0; k < 16; k++)
                        C(r, c0 + c) += A(r, k0 + k) * B(k0 + k, c0 + c);
}

void gepb_packed_b_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    float temp[16 * 16];

    for (size_t k0 = 0; k0 < K; k0 += 16) {
        for (size_t c0 = 0; c0 < M; c0 += 16) {
            for (size_t c = 0; c < 16; c++)
                for (size_t k = 0; k < 16; k++)
                    D(c, k) = B(k0 + k, c0 + c);

            for (size_t r = 0; r < N; r++)
                for (size_t c = 0; c < 16; c++)
                    for (size_t k = 0; k < 16; k++)
                        C(r, c0 + c) += A(r, k0 + k) * D(c, k);
        }
    }
}

void gepb_packed_a_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    float temp[16 * 16];
    float_aligned *A_packed = aligned_alloc(1024, N * 16 * sizeof(float));

    for (size_t k0 = 0; k0 < K; k0 += 16) {
        for (size_t r = 0; r < N; r++)
            for (size_t k = 0; k < 16; k++)
                A_packed[r * 16 + k] = A(r, k0 + k);

        for (size_t c0 = 0; c0 < M; c0 += 16) {
            for (size_t c = 0; c < 16; c++)
                for (size_t k = 0; k < 16; k++)
                    D(c, k) = B(k0 + k, c0 + c);

            for (size_t r = 0; r < N; r++)
                for (size_t c = 0; c < 16; c++)
                    for (size_t k = 0; k < 16; k++)
                        C(r, c0 + c) += A_packed[r * 16 + k] * D(c, k);
        }
    }

    free(A_packed);
}

void gepb_final_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    float temp[16 * 16];
    float_aligned *A_packed = aligned_alloc(1024, N * 16 * sizeof(float));
    float_aligned *C_packed = aligned_alloc(1024, N * N * sizeof(float));

    for (size_t k0 = 0; k0 < K; k0 += 16) {
        for (size_t r = 0; r < N; r++)
            for (size_t k = 0; k < 16; k++)
                A_packed[r * 16 + k] = A(r, k0 + k);

        for (size_t c0 = 0; c0 < M; c0 += 16) {
            for (size_t c = 0; c < 16; c++)
                for (size_t k = 0; k < 16; k++)
                    D(c, k) = B(k0 + k, c0 + c);

            float *ptr = C_packed + c0 * N;
            for (size_t r = 0; r < N; r++) {
                for (size_t c = 0; c < 16; c++) {
                    float x = 0;  // C(r, c0 + c)
                    for (size_t k = 0; k < 16; k++)
                        x += A_packed[r * 16 + k] * D(c, k);
                    *ptr++ += x;
                }
            }
        }
    }

    float *ptr = C_packed;
    for (size_t c0 = 0; c0 < M; c0 += 16)
        for (size_t r = 0; r < N; r++)
            for (size_t c = 0; c < 16; c++)
                C(r, c0 + c) = *ptr++;

    free(A_packed);
    free(C_packed);
}

void gepb_aligned_gemm(const size_t N, const size_t M, const size_t K, const float_aligned* __restrict__ lhs, const float_aligned* __restrict__ rhs, float_aligned* __restrict__ dst) {
    CHECK_ARGUMENT;

    float temp[16 * 16] __attribute__((aligned(1024)));
    float_aligned *A_packed = aligned_alloc(1024, N * 16 * sizeof(float));
    float_aligned *C_packed = aligned_alloc(1024, N * N * sizeof(float));

    for (size_t k0 = 0; k0 < K; k0 += 16) {
        for (size_t r = 0; r < N; r++)
            for (size_t k = 0; k < 16; k++)
                A_packed[r * 16 + k] = A(r, k0 + k);

        for (size_t c0 = 0; c0 < M; c0 += 16) {
            for (size_t c = 0; c < 16; c++)
                for (size_t k = 0; k < 16; k++)
                    D(c, k) = B(k0 + k, c0 + c);

            float *ptr = C_packed + c0 * N;
            for (size_t r = 0; r < N; r++) {
                for (size_t c = 0; c < 16; c++) {
                    float x = 0;  // C(r, c0 + c)
                    for (size_t k = 0; k < 16; k++)
                        x += A_packed[r * 16 + k] * D(c, k);
                    *ptr++ += x;
                }
            }
        }
    }

    float *ptr = C_packed;
    for (size_t c0 = 0; c0 < M; c0 += 16)
        for (size_t r = 0; r < N; r++)
            for (size_t c = 0; c < 16; c++)
                C(r, c0 + c) = *ptr++;

    free(A_packed);
    free(C_packed);
}

struct gepb_parallel_task {
    size_t N;
    size_t M;
    size_t K;
    size_t M_start;
    size_t M_end;
    const float *lhs;
    const float *rhs;
    float *C_packed;
};

void* gepb_parallel_gemm_inner(void *arg) {
    struct gepb_parallel_task *task = (struct gepb_parallel_task*)arg;
    const size_t N = task->N;
    const size_t M = task->M;
    const size_t K = task->K;
    const size_t M_start = task->M_start;
    const size_t M_end = task->M_end;
    const float *lhs = task->lhs;
    const float *rhs = task->rhs;
    float_aligned *C_packed = task->C_packed;

    float temp[16 * 16];
    float_aligned *A_packed = aligned_alloc(1024, N * 16 * sizeof(float));

    for (size_t k0 = 0; k0 < K; k0 += 16) {
        for (size_t r = 0; r < N; r++)
            for (size_t k = 0; k < 16; k++)
                A_packed[r * 16 + k] = A(r, k0 + k);

        for (size_t c0 = M_start; c0 < M_end; c0 += 16) {
            for (size_t c = 0; c < 16; c++)
                for (size_t k = 0; k < 16; k++)
                    D(c, k) = B(k0 + k, c0 + c);

            float *ptr = C_packed + c0 * N;
            for (size_t r = 0; r < N; r++) {
                for (size_t c = 0; c < 16; c++) {
                    float x = 0;  // C(r, c0 + c)
                    for (size_t k = 0; k < 16; k++)
                        x += A_packed[r * 16 + k] * D(c, k);
                    *ptr++ += x;
                }
            }
        }
    }

    free(A_packed);
    return NULL;
}

void gepb_parallel_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;

    float_aligned *C_packed = aligned_alloc(1024, N * M * sizeof(float));

    struct gepb_parallel_task tasks[TASK];
    pthread_t threads[TASK];

    for (size_t i = 0; i < TASK; i++) {
        tasks[i] = (struct gepb_parallel_task){
            .N = N,
            .M = M,
            .K = K,
            .M_start = M * i / 16 / TASK * 16,
            .M_end = M * (i + 1) / 16 / TASK * 16,
            .lhs = lhs,
            .rhs = rhs,
            // .C_packed = C_packed,
        };
    }
    tasks[TASK-1].M_end = M;

    for (size_t i = 0; i < TASK - 1; i++)
        pthread_create(&threads[i], NULL, gepb_parallel_gemm_inner, (void*)&tasks[i]);
    gepb_parallel_gemm_inner((void*)&tasks[TASK-1]);
    for (size_t i = 0; i < TASK - 1; i++)
        pthread_join(threads[i], NULL);

    float *ptr = C_packed;
    for (size_t c0 = 0; c0 < M; c0 += 16)
        for (size_t r = 0; r < N; r++)
            for (size_t c = 0; c < 16; c++)
                C(r, c0 + c) = *ptr++;

    free(C_packed);
}

 void plain_rkc_blocked_openmp_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
     const size_t M2 = M / 16 * 16;

     #pragma omp parallel for
     for (size_t r0 = 0; r0 < N; r0 += 16)
         for (size_t k0 = 0; k0 < K; k0 += 16)
             for (size_t r = 0; r < 16; r++)
                 for (size_t k = 0; k < 16; k++)
                     for (size_t c = 0; c < M2; c++)
                         C(r0 + r, c) += A(r0 + r, k0 + k) * B(k0 + k, c);
 }

void openblas_gemm(const size_t N, const size_t M, const size_t K, const float *lhs, const float *rhs, float *dst) {
    CHECK_ARGUMENT;
    cblas_sgemm(CblasRowMajor, CblasNoTrans, CblasNoTrans, N, M, K, 1.0f, lhs, K, rhs, M, 0.0f, dst, M);
}
