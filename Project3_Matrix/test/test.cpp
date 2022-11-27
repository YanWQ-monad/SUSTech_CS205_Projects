#include "test.h"
using std::initializer_list;

void newMatrixOn(matrix *mat, initializer_list<initializer_list<float>> list) {
    ASSERT_NE(mat, nullptr);

    size_t rows = list.size();
    if (rows == 0) {
        ASSERT_EQ(createMatrix(mat, (Size){0, 0}), MATRIX_SUCCESS);
        return;
    }

    size_t cols = list.begin()->size();
    float *buffer = new float[rows * cols];

    size_t p = 0;
    for (auto row : list) {
        ASSERT_EQ(row.size(), cols);
        for (float val : row)
            buffer[p++] = val;
    }

    ASSERT_EQ(createMatrix(mat, (Size){rows, cols}), MATRIX_SUCCESS);
    ASSERT_EQ(loadMatrixFromArray(mat, buffer), MATRIX_SUCCESS);
}

matrix newMatrix(initializer_list<initializer_list<float>> list) {
    matrix mat = {0, 0, nullptr};
    newMatrixOn(&mat, list);
    return mat;
}
