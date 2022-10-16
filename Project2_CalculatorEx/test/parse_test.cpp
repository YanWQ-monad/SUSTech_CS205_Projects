#include <iostream>
#include <gtest/gtest.h>
#include "parse.h"
#include "error.h"

TEST(ParsingTest, InvalidTest) {
    EXPECT_THROW(parse("1+", 0), ranged_error);
    EXPECT_THROW(parse("-", 0), ranged_error);
    EXPECT_THROW(parse("f[3][3]", 0), ranged_error);
    EXPECT_THROW(parse("1+1[3]", 0), ranged_error);
    EXPECT_THROW(parse("1+(1+1)[3]", 0), ranged_error);
    EXPECT_THROW(parse("1+(1", 0), ranged_error);
    EXPECT_THROW(parse("1+(1))", 0), ranged_error);
    EXPECT_THROW(parse("(1) (1)", 0), ranged_error);
    EXPECT_THROW(parse("(1)/7x", 0), ranged_error);
}
