# Google Benchmark & Test
#set(BENCHMARK_ENABLE_LTO ON)
set(BENCHMARK_ENABLE_LIBPFM ON)
set(BENCHMARK_ENABLE_TESTING OFF)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1)
FetchContent_Declare(
    benchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.7.0)

FetchContent_MakeAvailable(googletest benchmark)

set(BLA_VENDER OpenBLAS)
find_package(BLAS REQUIRED)

add_executable(matrix_benchmark benchmark.cpp)
target_link_libraries(matrix_benchmark benchmark::benchmark matrix ${BLAS_LIBRARIES})
target_compile_options(matrix_benchmark PRIVATE ${CXX_MY_FLAGS} -mcpu=apple-m1 -Ofast -lpthread)

include_directories(PRIVATE ${MatrixMultiplication_SOURCE_DIR}/src)

#add_executable(unittest token_test.cpp parse_test.cpp test.hpp number_test.cpp)
#target_link_libraries(unittest GTest::gtest_main libcalc)
#target_compile_options(unittest PRIVATE ${CXX_MY_FLAGS})
#include(GoogleTest)
#gtest_discover_tests(unittest)
