#include "test.h"

TEST(GetterSetterTest, GetterTest) {
    matrix mat;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{1.f, 2.f}, {-1.f, 4.f}});
    });

    EXPECT_FLOAT_EQ(getElementAt(&mat, (Position){1, 0}), -1.f);
    EXPECT_FLOAT_EQ(getElementAt(&mat, (Position){0, 1}), 2.f);

    EXPECT_TRUE(isnan(getElementAt(&mat, (Position){2, 1})));
    EXPECT_TRUE(isnan(getElementAt(&mat, (Position){0, 3})));
    EXPECT_TRUE(isnan(getElementAt(nullptr, (Position){0, 1})));

    matrix mat2 = {2, 2, nullptr};
    EXPECT_TRUE(isnan(getElementAt(&mat2, (Position){0, 1})));
}

TEST(GetterSetterTest, SetterTest) {
    matrix mat, expected;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&expected);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{1.f, 2.f}, {-1.f, 4.f}});
        expected = newMatrix({{1.f, 0.f}, {1.f, 4.f}});
    });

    EXPECT_EQ(setElementAt(&mat, (Position){1, 0}, 1.f), MATRIX_SUCCESS);
    EXPECT_EQ(setElementAt(&mat, (Position){0, 1}, 0.f), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&mat, &expected, 1e-5));

    EXPECT_EQ(setElementAt(&mat, (Position){2, 1}, 0.f), MATRIX_OUT_OF_RANGE);
    EXPECT_EQ(setElementAt(&mat, (Position){0, 3}, 0.f), MATRIX_OUT_OF_RANGE);

    EXPECT_EQ(setElementAt(nullptr, (Position){0, 1}, 0.f), MATRIX_NULL_MATRIX);

    matrix mat2 = {2, 2, nullptr};
    EXPECT_EQ(setElementAt(&mat2, (Position){0, 1}, 0.f), MATRIX_NULL_MATRIX);
}
