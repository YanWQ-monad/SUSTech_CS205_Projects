#include <gtest/gtest.h>
#include <initializer_list>
#include "matrix.h"

// RAII guard
// usage: defer _(nullptr, [&](...) { /* clean work here */ });
using guard = std::shared_ptr<void>;

void newMatrixOn(matrix *mat, std::initializer_list<std::initializer_list<float>> list);
matrix newMatrix(std::initializer_list<std::initializer_list<float>> list);
