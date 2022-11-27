#include <string>
#include "test.h"
using std::string;

TEST(IOTest, SaveLoadIdentityTest) {
    matrix mat, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{.1f, .2f}, {.3f, .4f}, {.5f, .6f}});
    });

    EXPECT_EQ(saveMatrix(&mat, "matrix.bin"), MATRIX_SUCCESS);
    EXPECT_EQ(loadMatrix(&output, "matrix.bin"), MATRIX_SUCCESS);
    EXPECT_TRUE(matrixEqual(&mat, &output, 1e-5f));
}

TEST(IOTest, SaveLoadErrorTest) {
    matrix mat, output;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
        deleteMatrix(&output);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{.1f, .2f}, {.3f, .4f}, {.5f, .6f}});
    });

    EXPECT_EQ(saveMatrix(&mat, ""), MATRIX_IO_FAILED);
    EXPECT_EQ(loadMatrix(&output, "not_exists_file"), MATRIX_IO_FAILED);

    FILE *f = fopen("incorrect.bin", "wb");
    size_t header[] = {1 << 30, 1 << 30};
    fwrite(header, sizeof(size_t), 2, f);
    fclose(f);

    EXPECT_EQ(loadMatrix(&output, "incorrect.bin"), MATRIX_ALLOC_FAILED);
}

TEST(PrintTest, SimplePrintTest) {
    matrix mat;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
    });
    ASSERT_NO_FATAL_FAILURE({
        mat = newMatrix({{.1f, .2f}, {.3f, .4f}, {.5f, .6f}});
    });

    testing::internal::CaptureStdout();
    printMatrix(&mat);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_EQ(R"([[          0.1,          0.2],
 [          0.3,          0.4],
 [          0.5,          0.6]]
)", output);
}

TEST(PrintTest, PrintTooBigTest) {
    matrix mat;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
    });
    ASSERT_NO_FATAL_FAILURE({
        ASSERT_EQ(createMatrix(&mat, (Size){10, 10}), MATRIX_SUCCESS);
        ASSERT_EQ(fillMatrix(&mat, 1.f), MATRIX_SUCCESS);
    });

    testing::internal::CaptureStdout();
    printMatrix(&mat);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("[      ...    ,      ...    ,      ...    ,   ...  ,      ...    ,      ...    ,      ...    ]"),
              string::npos);
}

TEST(PrintTest, PrintTooWideTest) {
    matrix mat;
    guard _(nullptr, [&](...) {
        deleteMatrix(&mat);
    });
    ASSERT_NO_FATAL_FAILURE({
        ASSERT_EQ(createMatrix(&mat, (Size){2, 10}), MATRIX_SUCCESS);
        ASSERT_EQ(fillMatrix(&mat, 1.f), MATRIX_SUCCESS);
    });

    testing::internal::CaptureStdout();
    printMatrix(&mat);
    string output = testing::internal::GetCapturedStdout();

    EXPECT_NE(output.find("..."), string::npos);
}
