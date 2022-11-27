#include "test.h"

TEST(ReduceTest, MinAndMaxTest) {
    matrix mat;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{1.f, 2.f}, {-1.f, 4.f}});
    });

    int error;
    EXPECT_FLOAT_EQ(minInMatrix(&mat, &error), -1.f);
    EXPECT_EQ(error, MATRIX_SUCCESS);

    EXPECT_FLOAT_EQ(maxInMatrix(&mat, nullptr), 4.f);
}

TEST(ReduceTest, MinErrorTest) {
    matrix mat = {0, 0, nullptr};

    int error;
    EXPECT_TRUE(isnan(minInMatrix(nullptr, &error)));
    EXPECT_EQ(error, MATRIX_NULL_MATRIX);

    EXPECT_TRUE(isnan(minInMatrix(nullptr, nullptr)));
    EXPECT_TRUE(isnan(minInMatrix(&mat, nullptr)));
}
