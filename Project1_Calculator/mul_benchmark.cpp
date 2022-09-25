#include <benchmark/benchmark.h>
#include <random>

#define main main2
#include "mul.cpp"
#undef main

using namespace std;

static random_device rd;
static mt19937 rng{rd()};
static uniform_int_distribution<> digit_distrib('0', '9');

static void generate_random_digits(string &s) {
    for (char &c : s)
        c = static_cast<char>(digit_distrib(rng));
    if (rng() & 1)
        s[0] = '-';
}

static void BM_BigIntegerParsing(benchmark::State &state) {
    string x(state.range(0), '0');
    for (auto _ : state) {
        state.PauseTiming();
        generate_random_digits(x);
        state.ResumeTiming();

        BigInteger integer(x);

        benchmark::DoNotOptimize(integer);
        benchmark::ClobberMemory();
    }
    state.SetComplexityN(state.range(0));
}

BENCHMARK(BM_BigIntegerParsing)
    ->RangeMultiplier(10)->Range(10, 10000000)->Complexity(benchmark::oN);

static void BM_BigDecimalParsing(benchmark::State &state) {
    const int64_t exponent_maximum = state.range(0) * state.range(0);
    uniform_int_distribution<int64_t> exponent_distrib(-exponent_maximum, exponent_maximum);

    string x(state.range(0), '0');
    for (auto _ : state) {
        state.PauseTiming();
        generate_random_digits(x);
        x[rng() % x.length()] = '.';
        string y = x + 'e' + to_string(exponent_distrib(rng));
        state.ResumeTiming();

        BigDecimal decimal(y);

        benchmark::DoNotOptimize(decimal);
        benchmark::ClobberMemory();
    }
    state.SetComplexityN(state.range(0));
}

BENCHMARK(BM_BigDecimalParsing)
    ->RangeMultiplier(10)->Range(10, 10000000)->Complexity(benchmark::oN);

static void BM_BigIntegerMultiply(benchmark::State &state) {
    string x(state.range(0), '0');
    for (auto _ : state) {
        state.PauseTiming();
        generate_random_digits(x);
        BigInteger integer(x);
        state.ResumeTiming();

        BigInteger result = integer * integer;

        benchmark::DoNotOptimize(result);
        benchmark::ClobberMemory();
    }
    state.SetComplexityN(state.range(0));
}

BENCHMARK(BM_BigIntegerMultiply)
    ->RangeMultiplier(10)->Range(10, 1000000)->Complexity(benchmark::oNLogN);

BENCHMARK_MAIN();
