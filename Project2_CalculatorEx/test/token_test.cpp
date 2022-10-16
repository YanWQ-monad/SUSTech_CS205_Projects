#include <gtest/gtest.h>
#include <iostream>
#include "token.h"
#include "error.h"

using std::string;
using std::vector;

void compare_token_vector(vector<Token> lhs, const vector<TokenContent> &rhs) {
    EXPECT_EQ(lhs.size(), rhs.size());
    for (size_t i = 0; i < lhs.size(); i++)
        EXPECT_EQ(lhs[i].content(), rhs[i]);
}

TEST(TokenizationTest, GeneralTest) {
    compare_token_vector(tokenize("5 + 2 * 3", 0),
                         vector<TokenContent>{BigDecimal("5"), '+', BigDecimal("2"), '*', BigDecimal("3")});
    compare_token_vector(tokenize("x+6", 0),
                         vector<TokenContent>{string("x"), '+', BigDecimal("6")});
    compare_token_vector(tokenize("(5 + 2 ) * 3", 0),
                         vector<TokenContent>{'(', BigDecimal("5"), '+', BigDecimal("2"), ')', '*', BigDecimal("3")});
    compare_token_vector(tokenize("func[2, x_]", 0),
                         vector<TokenContent>{string("func"), '[', BigDecimal("2"), ',', string("x_"), ']'});
}

TEST(TokenizationTest, InvalidTest) {
    EXPECT_THROW(tokenize("1e1.1 + 1", 0), ranged_error);
    EXPECT_THROW(tokenize("1 ' 2", 0), ranged_error);
}

TEST(TokenizationTest, RangeTest) {
    auto result = tokenize("func[ 233, x_]", 0);
    compare_token_vector(result, vector<TokenContent>{string("func"), '[', BigDecimal("233"), ',', string("x_"), ']'});
    EXPECT_EQ(result[0].range(), (TokenRange{0, 4, 0}));
    EXPECT_EQ(result[1].range(), (TokenRange{4, 5, 0}));
    EXPECT_EQ(result[2].range(), (TokenRange{6, 9, 0}));
    EXPECT_EQ(result[3].range(), (TokenRange{9, 10, 0}));
    EXPECT_EQ(result[4].range(), (TokenRange{11, 13, 0}));
    EXPECT_EQ(result[5].range(), (TokenRange{13, 14, 0}));
}
