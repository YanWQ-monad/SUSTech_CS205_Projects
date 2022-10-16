# Google Benchmark & Test
#set(BENCHMARK_ENABLE_LTO ON)
#set(BENCHMARK_ENABLE_TESTING OFF)
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1)
#FetchContent_Declare(
#    benchmark
#    GIT_REPOSITORY https://github.com/google/benchmark.git
#    GIT_TAG v1.7.0)

FetchContent_MakeAvailable(googletest)  # benchmark

#add_executable(mul_benchmark mul_benchmark.cpp)
#target_link_libraries(mul_benchmark PRIVATE benchmark::benchmark)
#target_compile_options(mul_benchmark PRIVATE ${CXX_MY_FLAGS})


include_directories(PRIVATE ${Calculator_SOURCE_DIR}/src)

add_executable(unittest token_test.cpp parse_test.cpp test.hpp number_test.cpp)
target_link_libraries(unittest GTest::gtest_main libcalc)
target_compile_options(unittest PRIVATE ${CXX_MY_FLAGS})
include(GoogleTest)
gtest_discover_tests(unittest)
