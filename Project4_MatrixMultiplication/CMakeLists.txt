cmake_minimum_required(VERSION 3.16)
project(MatrixMultiplication)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(C_MY_FLAGS -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 -Winit-self
    -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros
    -Wno-parentheses -fdiagnostics-show-option -mcpu=apple-m1 -Ofast)
set(CXX_MY_FLAGS -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization
    -Wformat=2 -Winit-self -Wmissing-include-dirs -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow
    -Wsign-promo -Wstrict-overflow=5 -Wundef -Wno-unused -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option -mcpu=apple-m1 -Ofast)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
