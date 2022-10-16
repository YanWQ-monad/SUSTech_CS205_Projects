cmake_minimum_required(VERSION 3.16)
project(CalculatorSrc CXX)

set(SRC parse.cpp node.cpp number.cpp token.cpp eval.cpp context.cpp constant.cpp)
set(SRC_H parse.h node.h number.h error.h token.h eval.h context.h constant.h)

add_library(libcalc STATIC ${SRC} ${SRC_H})
add_executable(calc ${SRC_H} main.cpp)
target_link_libraries(calc libcalc)

target_compile_options(libcalc PRIVATE ${CXX_MY_FLAGS})
target_compile_options(calc PRIVATE ${CXX_MY_FLAGS})
