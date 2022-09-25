cmake_minimum_required(VERSION 3.16)
project(CppTest)

set(CMAKE_CXX_STANDARD 17)

set(CXX_MY_FLAGS -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization
    -Wformat=2 -Winit-self -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow
    -Wsign-promo -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option)

if (CMAKE_BUILD_TYPE STREQUAL Release)
    set(CXX_MY_FLAGS -O2 ${CXX_MY_FLAGS})
else()
    set(CXX_MY_FLAGS -g ${CXX_MY_FLAGS})
    message("You are in development mode")
endif()

add_executable(mul mul.cpp)
add_library(mul_abi SHARED abi.cpp)
target_compile_options(mul PRIVATE ${CXX_MY_FLAGS})
target_compile_options(mul_abi PRIVATE ${CXX_MY_FLAGS})

configure_file(correctness_test.py correctness_test.py @ONLY)

add_executable(mul_alternative_1 mul_alternative_1.cpp)
add_executable(mul_alternative_2 mul_alternative_2.cpp)
target_compile_options(mul_alternative_1 PRIVATE -fwrapv)

enable_testing()
add_test(NAME PythonCorrectnessTest COMMAND python3 correctness_test.py)

function(simple_test name result target)
    add_test(NAME ${name} COMMAND ${target} ${ARGN})
    set_tests_properties(${name} PROPERTIES PASS_REGULAR_EXPRESSION ${result})
endfunction()
simple_test(SimpleTest1 "^2 \\* 3 = 6\n$" mul 2 3)
simple_test(SimpleTest2 "^3\\.1416 \\* 2 = 6\\.2832\n$" mul 3.1416 2)
simple_test(SimpleTest3 "^3\\.1416 \\* 0\\.02 = 0\\.062832\n$" mul 3.1416 2.0e-2)
simple_test(SimpleTest4 "^1234567890 \\* 1234567890 = 1524157875019052100\n$" mul 1234567890 1234567890)
simple_test(SimpleTest5 "^The input cannot be interpreted as numbers: not digit \\(0 to 9\\)\n$" mul a 2)
simple_test(SimpleTest6 "^USAGE: " mul --help)
simple_test(SimpleTest7 "You input less numbers than expected" mul 1)
simple_test(SimpleTest8 "^1\\.2345e\\+3 \\* 6\\.789e-2 = 8\\.381020e\\+1\n$" mul 1234.5 .06789 -s)
simple_test(SimpleTest9 "^1\\.234e\\+3 \\* 6\\.789e-2 = 8\\.381e\\+1\n$" mul 1234.5 0.06789 -s 3)

# Google Benchmark & Test
#set(BENCHMARK_ENABLE_LTO ON)
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

add_executable(mul_benchmark mul_benchmark.cpp)
target_link_libraries(mul_benchmark PRIVATE benchmark::benchmark)
target_compile_options(mul_benchmark PRIVATE ${CXX_MY_FLAGS})

add_executable(mul_test mul_test.cpp)
target_link_libraries(mul_test GTest::gtest_main)
target_compile_options(mul_test PRIVATE ${CXX_MY_FLAGS})
include(GoogleTest)
gtest_discover_tests(mul_test)
