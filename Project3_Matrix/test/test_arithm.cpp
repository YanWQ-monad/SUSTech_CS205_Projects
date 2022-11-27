#include "test.h"

TEST(PointwiseTest, AddTest) {
    matrix lhs, rhs, answer, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&lhs);
        deleteMatrix(&rhs);
        deleteMatrix(&answer);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        lhs = newMatrix({{.1f, .2f}, {.3f, .4f}});
        rhs = newMatrix({{.1f, .0f}, {.1f, .7f}});
        answer = newMatrix({{.2f, .2f}, {.4f, 1.1f}});
    });

    ASSERT_EQ(addMatrix(&output, &lhs, &rhs), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&answer, &output, 1e-5f));
}

TEST(PointwiseTest, SubTest) {
    matrix lhs, rhs, answer, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&lhs);
        deleteMatrix(&rhs);
        deleteMatrix(&answer);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        lhs = newMatrix({{.1f, .2f}, {.3f, .4f}});
        rhs = newMatrix({{.1f, .0f}, {.1f, .7f}});
        answer = newMatrix({{.0f, .2f}, {.2f, -.3f}});
    });

    ASSERT_EQ(subMatrix(&output, &lhs, &rhs), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&answer, &output, 1e-5f));
}

TEST(PointwiseTest, ParameterNullTest) {
    matrix mat, partial = {0, 0, nullptr}, output = partial;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&partial);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{.1f, .2f}, {.3f, .4f}});
    });

    EXPECT_EQ(addMatrix(nullptr, &mat, &mat), MATRIX_ARGUMENT_INVALID);
    EXPECT_EQ(addMatrix(&output, &partial, &mat), MATRIX_NULL_MATRIX);
    EXPECT_EQ(addMatrix(&output, nullptr, &mat), MATRIX_NULL_MATRIX);
    EXPECT_EQ(addMatrix(&output, &mat, &partial), MATRIX_NULL_MATRIX);
    EXPECT_EQ(addMatrix(&output, &mat, nullptr), MATRIX_NULL_MATRIX);
}

TEST(PointwiseTest, SizeMismatchedTest) {
    matrix lhs, rhs, output = {0, 0, nullptr};
    guard _(nullptr, [&](...) {
        deleteMatrix(&lhs);
        deleteMatrix(&rhs);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        lhs = newMatrix({{.1f, .2f}, {.3f, .4f}});
        rhs = newMatrix({{.1f, .0f, .8f}, {.1f, .7f, .5f}});
    });

    EXPECT_EQ(addMatrix(&output, &lhs, &rhs), MATRIX_SIZE_MISMATCHED);
}

TEST(MultiplicationTest, SimpleTest) {
    matrix lhs, rhs, answer, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&lhs);
        deleteMatrix(&rhs);
        deleteMatrix(&answer);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        lhs = newMatrix({{3.f, 4.f}, {8.f, 7.f}});
        rhs = newMatrix({{7.f, 2.f}, {4.f, 9.f}});
        answer = newMatrix({{37.f, 42.f}, {84.f, 79.f}});
    });

    EXPECT_EQ(mulMatrix(&output, &lhs, &rhs), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&answer, &output, 1e-5f));
}

TEST(MultiplicationTest, SizeMismatchedTest) {
    matrix lhs, rhs, output = {0, 0, nullptr};
    guard _(nullptr, [&](...) {
        deleteMatrix(&lhs);
        deleteMatrix(&rhs);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        lhs = newMatrix({{3.f, 4.f, 1.f}, {8.f, 7.f, 4.f}});
        rhs = newMatrix({{7.f, 2.f}, {4.f, 9.f}});
    });

    EXPECT_EQ(mulMatrix(&output, &lhs, &rhs), MATRIX_SIZE_MISMATCHED);
}

TEST(MultiplicationTest, ParameterNullTest) {
    matrix mat, partial = {0, 0, nullptr}, output = partial;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&partial);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{.1f, .2f}, {.3f, .4f}});
    });

    EXPECT_EQ(mulMatrix(nullptr, &mat, &mat), MATRIX_ARGUMENT_INVALID);
    EXPECT_EQ(mulMatrix(&output, &partial, &mat), MATRIX_NULL_MATRIX);
    EXPECT_EQ(mulMatrix(&output, nullptr, &mat), MATRIX_NULL_MATRIX);
    EXPECT_EQ(mulMatrix(&output, &mat, &partial), MATRIX_NULL_MATRIX);
    EXPECT_EQ(mulMatrix(&output, &mat, nullptr), MATRIX_NULL_MATRIX);
}
