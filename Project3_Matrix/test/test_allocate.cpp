#include <gmock/gmock.h>
#include "test.h"
using std::string;
using testing::HasSubstr;

TEST(CreateMatrixExTest, SimpleTest) {
    matrix *mat = createMatrixEx((Size){2, 2});
    EXPECT_NE(mat, nullptr);
    EXPECT_NE(mat->p_data, nullptr);
    deleteMatrixEx(mat);
}

TEST(CreateMatrixExTest, TooLargeTest) {
    Size size = { 1 << 30, 1 << 30 };

    testing::internal::CaptureStderr();
    EXPECT_EQ(createMatrixEx(size), nullptr);
    string output = testing::internal::GetCapturedStderr();

    EXPECT_THAT(output, HasSubstr("failed to allocate 4611686018427387904 bytes for the matrix"));
}

TEST(CreateMatrixTest, NullArgumentTest) {
    EXPECT_EQ(createMatrix(nullptr, (Size){2, 2}), MATRIX_ARGUMENT_INVALID);
}

TEST(CreateUnallocatedMatrixTest, SimpleTest) {
    matrix *mat = createUnallocatedMatrix();
    EXPECT_NE(mat, nullptr);
    EXPECT_EQ(mat->p_data, nullptr);
    free(mat);
}

TEST(DeleteMatrixTest, NullTest) {
    deleteMatrix(nullptr);  // assume safe
}
