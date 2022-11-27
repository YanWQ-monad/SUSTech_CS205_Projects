cmake_minimum_required(VERSION 3.16)
project(MatrixMultiplicationSrc CXX)

set(SRC matrix.c mul.c)
set(SRC_H matrix.h)

add_library(matrix STATIC ${SRC} ${SRC_H})

target_compile_options(matrix PRIVATE ${C_MY_FLAGS} -I/opt/homebrew/opt/openblas/include)
#target_link_libraries(matrix PUBLIC)
