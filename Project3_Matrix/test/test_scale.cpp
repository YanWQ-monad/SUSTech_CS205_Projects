#include "test.h"

TEST(ScaleTest, SubTest) {
    matrix original, answer, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&original);
        deleteMatrix(&answer);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        original = newMatrix({{.1f, .2f}, {.3f, .4f}});
        answer = newMatrix({{.0f, .1f}, {.2f, .3f}});
    });

    ASSERT_EQ(subMatrixScale(&output, &original, .1f), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&answer, &output, 1e-5f));
}

TEST(ScaleTest, MulTest) {
    matrix original, answer, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&original);
        deleteMatrix(&answer);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        original = newMatrix({{.1f, .2f}, {.3f, .4f}});
        answer = newMatrix({{.2f, .4f}, {.6f, .8f}});
    });

    ASSERT_EQ(mulMatrixScale(&output, &original, 2.f), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&answer, &output, 1e-5f));
}

TEST(ScaleTest, DivTest) {
    matrix original, answer, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&original);
        deleteMatrix(&answer);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        original = newMatrix({{.1f, .2f}, {.3f, .4f}});
        answer = newMatrix({{.05f, .1f}, {.15f, .2f}});
    });

    ASSERT_EQ(divMatrixScale(&output, &original, 2.f), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&answer, &output, 1e-5f));
}

TEST(ScaleTest, ParameterNullTest) {
    matrix mat, partial = {0, 0, nullptr}, output = partial;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&partial);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{.1f, .2f}, {.3f, .4f}});
    });

    EXPECT_EQ(addMatrixScale(nullptr, &mat, 1.f), MATRIX_ARGUMENT_INVALID);
    EXPECT_EQ(addMatrixScale(&output, &partial, 1.f), MATRIX_NULL_MATRIX);
    EXPECT_EQ(addMatrixScale(&output, nullptr, 1.f), MATRIX_NULL_MATRIX);
}
