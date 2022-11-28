cmake_minimum_required(VERSION 3.16)
project(MatrixLibrary C)

set(CMAKE_C_STANDARD 99)

set(CXX_MY_FLAGS -Wall -Wextra -pedantic -Wcast-align -Wcast-qual -Wdisabled-optimization -Wno-comment -Wformat=2
    -Winit-self -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wstrict-overflow=5 -Wundef -Wno-unused
    -Wno-variadic-macros -Wno-parentheses -fdiagnostics-show-option)

set(SRC matrix.c)
set(SRC_H matrix.h)

add_library(mat STATIC ${SRC} ${SRC_H})
add_library(mat_coverage STATIC ${SRC} ${SRC_H})
add_executable(example example.c ${SRC_H})

target_link_libraries(example mat)
target_compile_options(mat PRIVATE ${CXX_MY_FLAGS})
target_compile_options(mat_coverage PRIVATE ${CXX_MY_FLAGS} --coverage)

file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/**/*.gcda)

if (CMAKE_BUILD_TYPE MATCHES "^[Rr]elease")
    set(DOXYGEN_IN ${MatrixLibrary_SOURCE_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    find_package(Doxygen)
    if (DOXYGEN_FOUND)
        add_custom_target(doc_doxygen ALL
            COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
            COMMENT "Generating API documentation with Doxygen"
            VERBATIM)
    else (DOXYGEN_FOUND)
      message("Doxygen need to be installed to generate the doxygen documentation")
    endif (DOXYGEN_FOUND)
endif()

enable_testing()

add_subdirectory(test)
