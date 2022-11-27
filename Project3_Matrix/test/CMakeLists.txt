project(MatrixLibraryTest CXX)

# Google Test
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1)

FetchContent_MakeAvailable(googletest)

include_directories(PRIVATE ${MatrixLibrary_SOURCE_DIR})

add_executable(unittest test.h test.cpp test_arithm.cpp test_basic_op.cpp test_scale.cpp test_reduce.cpp test_getter.cpp test_io.cpp test_allocate.cpp)
target_link_libraries(unittest GTest::gtest_main gmock mat_coverage)
target_compile_options(unittest PRIVATE --coverage)
target_link_options(unittest PRIVATE --coverage)

include(GoogleTest)
gtest_discover_tests(unittest)

add_custom_target(coverage)
add_custom_command(TARGET coverage
        MAIN_DEPENDENCY unittest
        COMMAND ctest
        COMMAND pwd
        COMMAND lcov --rc lcov_branch_coverage=1 -c -d .. -o coverage.info_temp
        COMMAND lcov --rc lcov_branch_coverage=1 -e coverage.info_temp "*/matrix.*" -o coverage.info
        COMMAND genhtml --rc genhtml_branch_coverage=1 coverage.info -o coverage_report
)
