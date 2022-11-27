#include "test.h"

TEST(BasicOpTest, CloneTest) {
    matrix mat, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{1.f, 2.f}, {3.f, 4.f}});
    });

    EXPECT_EQ(copyMatrix(&output, &mat), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&mat, &output, 1e-5f));
}

TEST(BasicOpTest, CloneParameterNullTest) {
    matrix mat, partial = {0, 0, nullptr}, output = partial;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&partial);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{.1f, .2f}, {.3f, .4f}});
    });

    EXPECT_EQ(copyMatrix(nullptr, &mat), MATRIX_ARGUMENT_INVALID);
    EXPECT_EQ(copyMatrix(&output, &partial), MATRIX_NULL_MATRIX);
    EXPECT_EQ(copyMatrix(&output, nullptr), MATRIX_NULL_MATRIX);
}

TEST(BasicOpTest, JoinAlongRowsTest) {
    matrix lhs, rhs, answer, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&lhs);
        deleteMatrix(&rhs);
        deleteMatrix(&answer);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        lhs = newMatrix({{1.f, 2.f}, {3.f, 4.f}});
        rhs = newMatrix({{5.f, 6.f}, {7.f, 8.f}});
        answer = newMatrix({{1.f, 2.f}, {3.f, 4.f}, {5.f, 6.f}, {7.f, 8.f}});
    });

    EXPECT_EQ(joinMatrixAlongRows(&output, &lhs, &rhs), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&answer, &output, 1e-5f));
}

TEST(BasicOpTest, JoinAlongColumnsTest) {
    matrix lhs, rhs, answer, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&lhs);
        deleteMatrix(&rhs);
        deleteMatrix(&answer);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        lhs = newMatrix({{1.f, 2.f}, {3.f, 4.f}});
        rhs = newMatrix({{5.f, 6.f}, {7.f, 8.f}});
        answer = newMatrix({{1.f, 2.f, 5.f, 6.f}, {3.f, 4.f, 7.f, 8.f}});
    });

    EXPECT_EQ(joinMatrixAlongColumns(&output, &lhs, &rhs), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&answer, &output, 1e-5f));
}

TEST(BasicOpTest, JoinSizeMismatchedTest) {
    matrix mat_2x3, mat_3x2, output = {0, 0, nullptr};
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat_2x3);
        deleteMatrix(&mat_3x2);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat_2x3 = newMatrix({{1.f, 2.f, 3.f}, {3.f, 4.f, 5.f}});
        mat_3x2 = newMatrix({{5.f, 6.f}, {7.f, 8.f}, {-1.f, -2.f}});
    });

    EXPECT_EQ(joinMatrixAlongColumns(&output, &mat_2x3, &mat_3x2), MATRIX_SIZE_MISMATCHED);
    EXPECT_EQ(joinMatrixAlongRows(&output, &mat_2x3, &mat_3x2), MATRIX_SIZE_MISMATCHED);
}

TEST(BasicOpTest, JoinParameterNullTest) {
    matrix mat, partial = {0, 0, nullptr}, output = partial;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&partial);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{.1f, .2f}, {.3f, .4f}});
    });

    EXPECT_EQ(joinMatrixAlongColumns(nullptr, &mat, &mat), MATRIX_ARGUMENT_INVALID);
    EXPECT_EQ(joinMatrixAlongColumns(&output, &partial, &mat), MATRIX_NULL_MATRIX);
    EXPECT_EQ(joinMatrixAlongColumns(&output, nullptr, &mat), MATRIX_NULL_MATRIX);
    EXPECT_EQ(joinMatrixAlongColumns(&output, &mat, &partial), MATRIX_NULL_MATRIX);
    EXPECT_EQ(joinMatrixAlongColumns(&output, &mat, nullptr), MATRIX_NULL_MATRIX);

    EXPECT_EQ(joinMatrixAlongRows(nullptr, &mat, &mat), MATRIX_ARGUMENT_INVALID);
    EXPECT_EQ(joinMatrixAlongRows(&output, &partial, &mat), MATRIX_NULL_MATRIX);
    EXPECT_EQ(joinMatrixAlongRows(&output, nullptr, &mat), MATRIX_NULL_MATRIX);
    EXPECT_EQ(joinMatrixAlongRows(&output, &mat, &partial), MATRIX_NULL_MATRIX);
    EXPECT_EQ(joinMatrixAlongRows(&output, &mat, nullptr), MATRIX_NULL_MATRIX);
}

TEST(BasicOpTest, CuttingTest) {
    matrix original, answer, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&original);
        deleteMatrix(&answer);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        original = newMatrix({{1.f, 2.f, 3.f, 4.f}, {5.f, 6.f, 7.f, 8.f}, {9.f, 10.f, 11.f, 12.f}});
        answer = newMatrix({{6.f, 7.f}, {10.f, 11.f}});
    });

    EXPECT_EQ(cutMatrix(&output, &original, (Position){1, 1}, (Size){2, 2}), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&answer, &output, 1e-5f));
}

TEST(BasicOpTest, Cutting2Test) {
    matrix original, answer, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&original);
        deleteMatrix(&answer);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        original = newMatrix({{1.f, 2.f, 3.f, 4.f}, {5.f, 6.f, 7.f, 8.f}, {9.f, 10.f, 11.f, 12.f}});
        answer = newMatrix({{6.f, 7.f}, {10.f, 11.f}});
    });

    EXPECT_EQ(cutMatrix2(&output, &original, (Position){1, 1}, (Position){2, 2}), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&answer, &output, 1e-5f));
}

TEST(BasicOpTest, CuttingOutOfRangeTest) {
    matrix original, output = {0, 0, nullptr};
    guard _(nullptr, [&](...) {
        deleteMatrix(&original);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        original = newMatrix({{1.f, 2.f, 3.f, 4.f}, {5.f, 6.f, 7.f, 8.f}, {9.f, 10.f, 11.f, 12.f}});
    });

    EXPECT_EQ(cutMatrix(&output, &original, (Position){5, 5}, (Size){2, 2}), MATRIX_OUT_OF_RANGE);
    EXPECT_EQ(cutMatrix(&output, &original, (Position){1, 1}, (Size){3, 3}), MATRIX_OUT_OF_RANGE);
    EXPECT_EQ(cutMatrix(&output, &original, (Position){1, 1}, (Size){1, 5}), MATRIX_OUT_OF_RANGE);
    EXPECT_EQ(cutMatrix(&output, &original, (Position){1, 1}, (Size){5, 1}), MATRIX_OUT_OF_RANGE);

    EXPECT_EQ(cutMatrix2(&output, &original, (Position){1, 1}, (Position){0, 0}), MATRIX_OUT_OF_RANGE);
    EXPECT_EQ(cutMatrix2(&output, &original, (Position){1, 1}, (Position){2, 0}), MATRIX_OUT_OF_RANGE);
}

TEST(BasicOpTest, CutParameterNullTest) {
    matrix mat, partial = {0, 0, nullptr}, output = partial;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&partial);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{.1f, .2f}, {.3f, .4f}});
    });

    EXPECT_EQ(cutMatrix(nullptr, &mat, (Position){0, 0}, (Size){1, 1}), MATRIX_ARGUMENT_INVALID);
    EXPECT_EQ(cutMatrix(&output, &partial, (Position){0, 0}, (Size){1, 1}), MATRIX_NULL_MATRIX);
    EXPECT_EQ(cutMatrix(&output, nullptr, (Position){0, 0}, (Size){1, 1}), MATRIX_NULL_MATRIX);

    EXPECT_EQ(cutMatrix2(nullptr, &mat, (Position){0, 0}, (Position){0, 0}), MATRIX_ARGUMENT_INVALID);
    EXPECT_EQ(cutMatrix2(&output, &partial, (Position){0, 0}, (Position){0, 0}), MATRIX_NULL_MATRIX);
    EXPECT_EQ(cutMatrix2(&output, nullptr, (Position){0, 0}, (Position){0, 0}), MATRIX_NULL_MATRIX);
}

TEST(BasicOpTest, FillTest) {
    matrix original, answer;
    guard _(nullptr, [&](...) {
        deleteMatrix(&original);
        deleteMatrix(&answer);
    });
    ASSERT_NO_FATAL_FAILURE({
        ASSERT_EQ(createMatrix(&original, (Size){3, 3}), MATRIX_SUCCESS);
        answer = newMatrix({{.5f, .5f, .5f}, {.5f, .5f, .5f}, {.5f, .5f, .5f}});
    });

    fillMatrix(&original, .5f);

    EXPECT_TRUE(matrixEqual(&answer, &original, 1e-5f));
}

TEST(BasicOpTest, FillWithTest) {
    matrix original, identity, ones, zeros;
    guard _(nullptr, [&](...) {
        deleteMatrix(&original);
        deleteMatrix(&identity);
        deleteMatrix(&ones);
        deleteMatrix(&zeros);
    });
    ASSERT_NO_FATAL_FAILURE({
        ASSERT_EQ(createMatrix(&original, (Size){3, 3}), MATRIX_SUCCESS);
        identity = newMatrix({{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}});
        ones = newMatrix({{1.f, 1.f, 1.f}, {1.f, 1.f, 1.f}, {1.f, 1.f, 1.f}});
        zeros = newMatrix({{0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}, {0.f, 0.f, 0.f}});
    });

    EXPECT_EQ(fillMatrixWith(&original, fill_identity), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&identity, &original, 1e-5f));

    EXPECT_EQ(fillMatrixWith(&original, fill_ones), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&ones, &original, 1e-5f));

    EXPECT_EQ(fillMatrixWith(&original, fill_zeros), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&zeros, &original, 1e-5f));
}

TEST(BasicOpTest, FillParameterNullTest) {
    matrix mat, partial = {0, 0, nullptr}, output = partial;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&partial);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{.1f, .2f}, {.3f, .4f}});
    });

    EXPECT_EQ(fillMatrix(&partial, 1.f), MATRIX_NULL_MATRIX);
    EXPECT_EQ(fillMatrix(nullptr, 1.f), MATRIX_NULL_MATRIX);

    EXPECT_EQ(fillMatrixWith(&partial, fill_identity), MATRIX_NULL_MATRIX);
    EXPECT_EQ(fillMatrixWith(nullptr, fill_identity), MATRIX_NULL_MATRIX);
    EXPECT_EQ(fillMatrixWith(&mat, nullptr), MATRIX_ARGUMENT_INVALID);
}

TEST(BasicOpTest, EqualTest) {
    matrix mat, mat2, mat_2x2, partial = {0, 0, nullptr};
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&mat2);
        deleteMatrix(&mat_2x2);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}});
        mat2 = newMatrix({{1.f, 1.f, 0.f}, {0.f, 1.f, 0.f}, {0.f, 0.f, 1.f}});
        mat_2x2 = newMatrix({{1.f, 0.f}, {0.f, 1.f}});
    });

    EXPECT_TRUE(matrixEqual(&mat, &mat, 1e-5f));
    EXPECT_FALSE(matrixEqual(&mat, &mat2, 1e-5f));

    EXPECT_FALSE(matrixEqual(&mat, nullptr, 1e-5f));
    EXPECT_FALSE(matrixEqual(nullptr, &mat, 1e-5f));
    EXPECT_FALSE(matrixEqual(nullptr, nullptr, 1e-5f));

    EXPECT_FALSE(matrixEqual(&mat, &partial, 1e-5f));
    EXPECT_FALSE(matrixEqual(&partial, &mat, 1e-5f));
    EXPECT_FALSE(matrixEqual(&partial, &partial, 1e-5f));

    EXPECT_FALSE(matrixEqual(&mat, &mat_2x2, 1e-5f));
}
