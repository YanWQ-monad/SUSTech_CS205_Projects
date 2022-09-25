#include <gtest/gtest.h>
#include <complex>
#include <random>

#define main main2
#include "mul.cpp"
#undef main

using namespace std;

static random_device rd;
static mt19937 rng{rd()};

static string remove_prefix(string s, char c) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [c](char ch) { return ch != c; }));
    return s;
}

static string big_integer_string(const BigInteger &integer) {
    char sign = integer.is_positive() ? '+' : '-';
    return string(1, sign) + remove_prefix(integer.get_number_string(), '0');
}

static string big_decimal_string(const BigDecimal &decimal) {
    ostringstream ss;
    ss << decimal;
    return ss.str();
}

TEST(FFTContextTest, IdentityTest) {
    constexpr int kSize = 1024;
    uniform_int_distribution<> distrib(0, 1 << 20);

    FFTContext context(kSize);
    vector<complex<double>> original(context.n_);
    generate(original.begin(), original.end(), [&distrib]() { return distrib(rng); });
    vector<complex<double>> data = original;

    context.dft(data);
    context.inverse_dft(data);

    auto lhs = original.begin(), rhs = data.begin();
    for (; lhs != original.end(); ++lhs, ++rhs) {
        EXPECT_NEAR((*rhs).real(), (*lhs).real(), 1e-9);
        EXPECT_NEAR((*rhs).imag(), 0, 1e-9);
    }
}

TEST(BigIntegerTest, ParsingTest) {
    EXPECT_EQ(big_integer_string(BigInteger("1")), "+1");
    EXPECT_EQ(big_integer_string(BigInteger("-1")), "-1");
    EXPECT_EQ(big_integer_string(BigInteger("114514")), "+114514");
    EXPECT_EQ(big_integer_string(BigInteger("-00012345")), "-12345");
}

TEST(BigDecimalTest, ParsingTest) {
    EXPECT_EQ(big_decimal_string(BigDecimal("12345.6789")), "12345.6789");
    EXPECT_EQ(big_decimal_string(BigDecimal("0.0000000000000001")), "0.0000000000000001");
    EXPECT_EQ(big_decimal_string(BigDecimal(".6789")), "0.6789");
    EXPECT_EQ(big_decimal_string(BigDecimal("123e+6")), "123000000");
    EXPECT_EQ(big_decimal_string(BigDecimal("123e-6")), "0.000123");
    EXPECT_EQ(big_decimal_string(BigDecimal("-5432.1e-01")), "-543.21");
    EXPECT_EQ(big_decimal_string(BigDecimal("-0")), "0");
    EXPECT_EQ(big_decimal_string(BigDecimal("0.000000e-123456")), "0");
}

TEST(BigDecimalTest, SanityTest) {
    EXPECT_EQ(big_decimal_string(BigDecimal("+-54.e+00000000000000005")), "-5400000");
    EXPECT_EQ(big_decimal_string(BigDecimal(".23456e-1")), "0.023456");
    EXPECT_EQ(big_decimal_string(BigDecimal(".")), "0");
}

TEST(BigDecimalTest, InvalidInputTest) {
    EXPECT_THROW({ BigDecimal("victorica"); }, number_parse_error);
    EXPECT_THROW({ BigDecimal("1234e1000000000000000000000"); }, number_parse_error);
    EXPECT_THROW({ BigDecimal("12.34e11.11"); }, number_parse_error);
    EXPECT_THROW({ BigDecimal("中文"); }, number_parse_error);
}

TEST(BigDecimalTest, MultiplicationTest) {
    BigDecimal lhs("-99.8"), rhs("10.17");
    EXPECT_EQ(big_decimal_string(lhs * rhs), "-1014.966");

    EXPECT_EQ(big_decimal_string(BigDecimal("-1017") * BigDecimal("-996")), "1012932");
    EXPECT_EQ(big_decimal_string(BigDecimal("-0") * BigDecimal("+0")), "0");
    EXPECT_EQ(big_decimal_string(BigDecimal("-0") * BigDecimal("1234.5")), "0");
}
