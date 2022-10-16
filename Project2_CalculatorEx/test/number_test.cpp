#include <gtest/gtest.h>
#include "test.hpp"
#include "error.h"
#include "constant.h"

TEST(DecimalTest, ParsingTest) {
    EXPECT_EQ(big_decimal_string(BigDecimal("12345.6789")), "12345.6789");
    EXPECT_EQ(big_decimal_string(BigDecimal("0.0000000000000001")), "0.0000000000000001");
    EXPECT_EQ(big_decimal_string(BigDecimal(".6789")), "0.6789");
    EXPECT_EQ(big_decimal_string(BigDecimal("123e+6")), "123000000");
    EXPECT_EQ(big_decimal_string(BigDecimal("123e-6")), "0.000123");
    EXPECT_EQ(big_decimal_string(BigDecimal("-5432.1e-01")), "-543.21");
    EXPECT_EQ(big_decimal_string(BigDecimal("-0")), "0");
    EXPECT_EQ(big_decimal_string(BigDecimal("0.000000e-123456")), "0");
    EXPECT_EQ(big_decimal_string(BigDecimal(".23456e-1")), "0.023456");
    EXPECT_EQ(big_decimal_string(BigDecimal(".")), "0");
}

TEST(DecimalTest, InvalidTest) {
    EXPECT_THROW({ BigDecimal("+-54.e+00000000000000005"); }, number_parse_error);
    EXPECT_THROW({ BigDecimal("victorica"); }, number_parse_error);
    EXPECT_THROW({ BigDecimal("1234e1000000000000000000000"); }, number_parse_error);
    EXPECT_THROW({ BigDecimal("12.34e11.11"); }, number_parse_error);
    EXPECT_THROW({ BigDecimal("中文"); }, number_parse_error);
}

TEST(DecimalTest, RoundScaleTest) {
    const auto rounded = [](auto d, size_t scale) { d.round_by_scale(scale); return d; };
    EXPECT_EQ(rounded(BigDecimal("1.14514"), 4), BigDecimal("1.1451"));
    EXPECT_EQ(rounded(BigDecimal("1.14514"), 2), BigDecimal("1.15"));
    EXPECT_EQ(rounded(BigDecimal("0.999999"), 5), BigDecimal("1"));
    EXPECT_EQ(rounded(BigDecimal("0.987654"), 5), BigDecimal("0.98765"));
    EXPECT_EQ(rounded(BigDecimal("0.012345"), 5), BigDecimal("0.01235"));
}

TEST(DecimalTest, FloorTest) {
    const auto floor = [](auto d) { d.drop_decimal(); return d; };
    EXPECT_EQ(floor(BigDecimal("1.14514")), BigDecimal("1"));
    EXPECT_EQ(floor(BigDecimal("1.9")), BigDecimal("1"));
    EXPECT_EQ(floor(BigDecimal("100.3")), BigDecimal("100"));
}

TEST(DecimalTest, AddTest) {
    EXPECT_EQ(BigDecimal("123") + BigDecimal("456"), BigDecimal("579"));
    EXPECT_EQ(BigDecimal("123.456") + BigDecimal("0.00001"), BigDecimal("123.45601"));
    EXPECT_EQ(BigDecimal("0.1") + BigDecimal("10"), BigDecimal("10.1"));
    EXPECT_EQ(BigDecimal("-0.1") + BigDecimal("0.1"), BigDecimal("0"));
    EXPECT_EQ(BigDecimal("-0.123") + BigDecimal("1"), BigDecimal("0.877"));
    EXPECT_EQ(BigDecimal("514") + BigDecimal("-114"), BigDecimal("400"));
    EXPECT_EQ(BigDecimal("444109631065506") + BigDecimal("976109770353748"), BigDecimal("1420219401419254"));
    EXPECT_EQ(BigDecimal("-496322471723981") + BigDecimal("482078777920624"), BigDecimal("-14243693803357"));
    EXPECT_EQ(BigDecimal("943047228") + BigDecimal("-373.97859"), BigDecimal("943046854.02141"));
}

TEST(DecimalTest, SubTest) {
    EXPECT_EQ(BigDecimal("114") - BigDecimal("514"), BigDecimal("-400"));
    EXPECT_EQ(BigDecimal("114") - BigDecimal("0.514"), BigDecimal("113.486"));
    EXPECT_EQ(BigDecimal("0.001") - BigDecimal("123"), BigDecimal("-122.999"));
    EXPECT_EQ(BigDecimal("0.5") - BigDecimal("0.5"), BigDecimal("0"));
    EXPECT_EQ(BigDecimal("123") - BigDecimal("0"), BigDecimal("123"));
    EXPECT_EQ(BigDecimal("320931062211474") - BigDecimal("931807828952597"), BigDecimal("-610876766741123"));
    EXPECT_EQ(BigDecimal("-605610756402905") - BigDecimal("994013215970185"), BigDecimal("-1599623972373090"));
    EXPECT_EQ(BigDecimal("760675598401039") - BigDecimal("352771784976631"), BigDecimal("407903813424408"));
}

TEST(DecimalTest, MultiplicationTest) {
    EXPECT_EQ(BigDecimal("-99.8") * BigDecimal("10.17"), BigDecimal("-1014.966"));
    EXPECT_EQ(BigDecimal("-1017") * BigDecimal("-996"), BigDecimal("1012932"));
    EXPECT_EQ(BigDecimal("-0") * BigDecimal("+0"), BigDecimal("0"));
    EXPECT_EQ(BigDecimal("-0") * BigDecimal("1234.5"), BigDecimal("0"));
    EXPECT_EQ(BigDecimal("331145390") * BigDecimal("583296797"), BigDecimal("193156045328315830"));
    EXPECT_EQ(BigDecimal("-463318767") * BigDecimal("836911256"), BigDecimal("-387756691218341352"));
    EXPECT_EQ(BigDecimal("-392864621") * BigDecimal("-249058356"), BigDecimal("97846216636823076"));
}

TEST(DecimalTest, DivideWithDivisibleTest) {
    EXPECT_EQ(BigDecimal("4") / BigDecimal("1"), BigDecimal("4"));
    EXPECT_EQ(BigDecimal("4") / BigDecimal("2"), BigDecimal("2"));
    EXPECT_EQ(BigDecimal("0") / BigDecimal("2"), BigDecimal("0"));
    EXPECT_EQ(BigDecimal("-114514") / BigDecimal("-31"), BigDecimal("3694"));
    EXPECT_EQ(BigDecimal("114514") / BigDecimal("-1847"), BigDecimal("-62"));
}

TEST(DecimalTest, DivideWithIndivisibleTest) {
    EXPECT_EQ(BigDecimal("2").div_with_scale(BigDecimal("3"), 5), BigDecimal("0.66667"));
    EXPECT_EQ(BigDecimal("4").div_with_scale(BigDecimal("3"), 5), BigDecimal("1.33333"));
    EXPECT_EQ(BigDecimal("2").div_with_scale(BigDecimal("7"), 10), BigDecimal("0.2857142857"));
    EXPECT_EQ(BigDecimal("1").div_with_scale(BigDecimal("1.0000000001"), 5), BigDecimal("1.0"));
    EXPECT_EQ(BigDecimal("2398048012").div_with_scale(BigDecimal("3425309249037"), 100), BigDecimal("0.0007000967905815199632938263091579117230711559808264095306243757050524047730771946493512486346525507"));
}

TEST(DecimalTest, SimpleDivideTest) {
    EXPECT_EQ(BigDecimal("4").simple_div_with_scale(1, 10), BigDecimal("4"));
    EXPECT_EQ(BigDecimal("4").simple_div_with_scale(2, 10), BigDecimal("2"));
    EXPECT_EQ(BigDecimal("0").simple_div_with_scale(2, 10), BigDecimal("0"));
    EXPECT_EQ(BigDecimal("-114514").simple_div_with_scale(31, 10), BigDecimal("-3694"));
    EXPECT_EQ(BigDecimal("114514").simple_div_with_scale(1847, 10), BigDecimal("62"));

    EXPECT_EQ(BigDecimal("2").simple_div_with_scale(3, 5), BigDecimal("0.66667"));
    EXPECT_EQ(BigDecimal("4").simple_div_with_scale(3, 5), BigDecimal("1.33333"));
    EXPECT_EQ(BigDecimal("2").simple_div_with_scale(7, 10), BigDecimal("0.2857142857"));
    EXPECT_EQ(BigDecimal("2398048012").simple_div_with_scale(3425309249037, 100), BigDecimal("0.0007000967905815199632938263091579117230711559808264095306243757050524047730771946493512486346525507"));
}

TEST(DecimalTest, DivideByZeroTest) {
    EXPECT_THROW(BigDecimal("1") / BigDecimal("0"), runtime_error);
    EXPECT_THROW(BigDecimal("1") % BigDecimal("0"), runtime_error);
}

TEST(DecimalTest, ModTest) {
    EXPECT_EQ(BigDecimal("5") % BigDecimal("3"), BigDecimal("2"));
    EXPECT_EQ(BigDecimal("1") % BigDecimal("3"), BigDecimal("1"));
    EXPECT_EQ(BigDecimal("0.7") % BigDecimal("0.3"), BigDecimal("0.1"));
}

TEST(DecimalTest, CompareTest) {
    EXPECT_LE(BigDecimal("123"), BigDecimal("456"));
    EXPECT_LE(BigDecimal("-100"), BigDecimal("1"));
    EXPECT_LE(BigDecimal("0"), BigDecimal("123"));
    EXPECT_LE(BigDecimal("114.514"), BigDecimal("114.52"));
}

TEST(DecimalTest, ZeroTests) {
    for (int i = 0; i < 100; i++) {
        BigDecimal d = get_decimal(100);
        BigDecimal e = BIG_DECIMAL_ZERO - d;

        // everything except `positive_` should be identical
        EXPECT_EQ(d.mantissa().digits(), e.mantissa().digits());
        EXPECT_EQ(d.exponent(), e.exponent());
        EXPECT_EQ(d.positive(), !e.positive());

        BigDecimal z = d + e;
        EXPECT_EQ(z, BIG_DECIMAL_ZERO);
        EXPECT_EQ(z.mantissa().digits().empty(), true);

        EXPECT_EQ(d + BIG_DECIMAL_ZERO, d);
        EXPECT_EQ(d - BIG_DECIMAL_ZERO, d);
    }
}
