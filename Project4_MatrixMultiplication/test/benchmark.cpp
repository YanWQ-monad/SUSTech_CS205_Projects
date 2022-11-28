#include <benchmark/benchmark.h>
#include <random>
#include <utility>

#include "matrix.h"

using namespace std;

static random_device rd;
static mt19937 rng{rd()};
static uniform_real_distribution<float> distrib(-10.0f, 10.0f);

class MatrixWrapper {
    size_t size;
    float *p_data;

 public:
    MatrixWrapper(const size_t rows, const size_t cols) {
        size = rows * cols;
        p_data = static_cast<float*>(aligned_alloc(1024, size * sizeof(p_data[0])));
        zero();
    }

    ~MatrixWrapper() {
        free(p_data);
    }

    void randomize() {
        for (size_t i = 0; i < size; i++)
            p_data[i] = distrib(rng);
    }

    void zero() {
        for (size_t i = 0; i < size; i++)
            p_data[i] = 0;
    }

    float* data() {
        return p_data;
    }
};

class Executor {
    // using func_t = function<matrix *(const matrix *, const matrix *)>;
    using func_t = function<void(size_t, size_t, size_t, const float*, const float*, float*)>;
    func_t func;

 public:
    explicit Executor(func_t func) : func(std::move(func)) {}

    void execute(benchmark::State &state) {
        const size_t N = state.range(0);

        MatrixWrapper lhs(N, N), rhs(N, N), dst(N, N);
        lhs.randomize();
        rhs.randomize();

        for (auto _ : state) {
            func(N, N, N, lhs.data(), rhs.data(), dst.data());
            benchmark::DoNotOptimize(dst.data()[rng() % (N * N)]);
            benchmark::ClobberMemory();
        }
    }
};

// extern float *A_packed, *C_packed;
// static void DoSetup(const benchmark::State& state) {
//     const size_t N = state.range(0);

//     A_packed = static_cast<float*>(aligned_alloc(1024, N * 16 * sizeof(float)));
//     C_packed = static_cast<float*>(aligned_alloc(1024, N * N * sizeof(float)));
// }

// static void DoTeardown(const benchmark::State& state) {
//     free(A_packed);
//     free(C_packed);
// }

#define ADD_BENCHMARK(FUNC, BENCHMARK_NAME) \
    static void BENCHMARK_NAME(benchmark::State &state) {  \
        Executor(FUNC).execute(state);  \
        state.SetComplexityN(state.range(0));  \
    }  \
    BENCHMARK(BENCHMARK_NAME)->Arg(16)->Arg(128)->Arg(1 << 10)->Arg(1 << 13)->Complexity(benchmark::oNCubed);
    // BENCHMARK(BENCHMARK_NAME)->DenseRange(16, 1024, 16)->Arg(1 << 11)->Arg(1 << 12)->Arg(1 << 13)->Setup(DoSetup)->Teardown(DoTeardown)->Complexity(benchmark::oNCubed);

ADD_BENCHMARK(gepb_gemm, BM_GEPBGemm)
ADD_BENCHMARK(gepb_packed_b_gemm, BM_GEPBPackedBGemm)
ADD_BENCHMARK(gepb_packed_a_gemm, BM_GEPBPackedAGemm)
ADD_BENCHMARK(gepb_final_gemm, BM_GEPBFinalGemm)
ADD_BENCHMARK(gepb_aligned_gemm, BM_GEPBAlignedGemm)

ADD_BENCHMARK(openblas_gemm, BM_OpenBLAS)

ADD_BENCHMARK(plain_gemm, BM_Plain)
ADD_BENCHMARK(plain_rkc_gemm, BM_PlainRkc)
ADD_BENCHMARK(plain_krc_gemm, BM_Plain_krc)
ADD_BENCHMARK(plain_kcr_gemm, BM_Plain_kcr)
ADD_BENCHMARK(plain_crk_gemm, BM_Plain_crk)
ADD_BENCHMARK(plain_ckr_gemm, BM_Plain_ckr)
ADD_BENCHMARK(plain_rkc_blocked_gemm, BM_PlainRkcBlocked)

// ADD_BENCHMARK(gepb_parallel_gemm, BM_GEPBParallelGemm)

BENCHMARK_MAIN();
