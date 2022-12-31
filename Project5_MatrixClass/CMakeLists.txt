cmake_minimum_required(VERSION 3.21)
project(MatrixClass)

set(CMAKE_CXX_STANDARD 17)

set(CXX_MY_FLAGS -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization
    -Wformat=2 -Winit-self -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow
    -Wsign-promo -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option)

add_executable(matrix_example example.cpp)
target_compile_options(matrix_example PRIVATE ${CXX_MY_FLAGS})

find_package(BLAS REQUIRED)
if (BLAS_FOUND)
    find_path(BLAS_INCLUDE_DIRS cblas.h
            /usr/include
            /usr/local/include
            /opt/homebrew/opt/openblas/include
            $ENV{BLAS_HOME}/include)

    target_include_directories(matrix_example PUBLIC ${BLAS_INCLUDE_DIRS})
    target_link_libraries(matrix_example ${BLAS_LIBRARIES})
endif()
