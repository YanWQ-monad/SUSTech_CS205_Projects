#include <algorithm>
#include <iostream>
#include <utility>

#include "constant.h"
#include "error.h"
#include "eval.h"

using std::function;
using std::min;
using std::to_string;

bool should_newtons_end(const BigDecimal &lhs, const BigDecimal &rhs, const size_t scale) {
    if (lhs.exponent() != rhs.exponent() || lhs.mantissa().digits().size() != rhs.mantissa().digits().size())
        return false;

    // since sometimes the x will jitter in the last digit
    // so, we just compare them without caring about the last digit
    size_t len = min(lhs.mantissa().digits().size(), 1UL);

    // however, if the last digit is not at the end of scale (like number "2" and "3"), then do not ignore
    if (-lhs.exponent() <= static_cast<int64_t>(scale))
        len = 0;

    return equal(lhs.mantissa().digits().begin() + len, lhs.mantissa().digits().end(),
                 rhs.mantissa().digits().begin() + len, rhs.mantissa().digits().end());
}

BigDecimal newtons_method(const function<BigDecimal(const BigDecimal&)>& formula,
                          BigDecimal initial, const size_t scale) {
    BigDecimal x = std::move(initial);
    while (true) {
        BigDecimal y = formula(x);
        y.round_by_scale(scale + kExtraScale);
        if (should_newtons_end(x, y, scale))
            break;
        x = std::move(y);
    }
    return x;
}

BigDecimal pow(BigDecimal x, BigDecimal y, const size_t required_scale) {
    size_t scale = required_scale + kExtraScale;
    BigDecimal result = BIG_DECIMAL_ONE;

    y.drop_decimal();
    while (!y.is_zero()) {
        // check the units digit is available (otherwise it means 0) and it's odd
        if (y.exponent() == 0 && y.mantissa().digits()[0] % 2 == 1)
            result = result * x;
        x = x * x;
        x.round_by_scale(scale);

        // floor div
        y = y.simple_div_with_scale(2, 1);
        y.drop_decimal();
    }

    return result;
}

BigDecimal sqrt(const BigDecimal &x, const size_t scale) {
    if (x.is_zero())
        return BIG_DECIMAL_ZERO;
    if (x < BIG_DECIMAL_ZERO)
        throw runtime_error("try to sqrt a negative number");

    size_t inv_scale = x.most_significant_exponent() + scale;
    BigDecimal half_x = x * BIG_DECIMAL_HALF;
    BigDecimal initial = BigDecimal(BigInteger({1}), - (x.most_significant_exponent() + 1) / 2, true);

    // y <- y * (3/2 - (x/2) * y^2)
    BigDecimal inv_sqrt = newtons_method([&half_x](auto &y) { return y * (BIG_DECIMAL_THREEHALFS - half_x * y * y); },
                                         std::move(initial), inv_scale);

    BigDecimal result = BIG_DECIMAL_ONE.div_with_scale(inv_sqrt, scale + kExtraScale);

    // do an extra Newton's Iteration
    result = BIG_DECIMAL_HALF * (result + x.div_with_scale(result, scale + kExtraScale));
    result.round_by_scale(scale);
    return result;
}

BigDecimal trigonometric_functions_taylor(const BigDecimal &x2, BigDecimal first, uint64_t k, const size_t scale) {
    BigDecimal result = BIG_DECIMAL_ZERO;
    BigDecimal term = std::move(first);
    while (!term.is_zero()) {
        result = result + term;
        term = (term * x2).simple_div_with_scale(k * (k + 1), scale + kExtraScale);
        k += 2;
    }
    result.round_by_scale(scale);
    return result;
}

// sin[x] = x - x^3/6 + x^5/120 - ...
BigDecimal sin(const BigDecimal &x, const size_t scale) {
    return trigonometric_functions_taylor(- x * x, x, 2, scale);
}

// cos[x] = 1 - x^2/2 + x^4/24 - ...
BigDecimal cos(const BigDecimal &x, const size_t scale) {
    return trigonometric_functions_taylor(- x * x, BIG_DECIMAL_ONE, 1, scale);
}

// arctan[x] = arctan[c] + arctan[(x-c)/(1+cx)], for small c (here c = 0.2)
// formula: arctan[x] = x - x^3/3 + x^5/5 - ...
BigDecimal arctan(BigDecimal x, const size_t required_scale) {
    if (x < BIG_DECIMAL_ZERO)
        return -arctan(-x, required_scale);

    size_t scale = required_scale + kExtraScale;
    BigDecimal arctan_02 = BIG_DECIMAL_ZERO;
    size_t f = 0;

    if (x > BIG_DECIMAL_ZERO_TWO) {
        arctan_02 = arctan(BIG_DECIMAL_ZERO_TWO, scale);
        while (x > BIG_DECIMAL_ZERO_TWO) {
            f++;
            x = (x - BIG_DECIMAL_ZERO_TWO).div_with_scale(BIG_DECIMAL_ONE + x * BIG_DECIMAL_ZERO_TWO, scale);
        }
    }
    arctan_02 = arctan_02.simple_mul(f, 0);

    BigDecimal result = BIG_DECIMAL_ZERO;
    BigDecimal x_square = - x * x;
    BigDecimal term = x;
    uint64_t k = 1;
    while (!term.is_zero()) {
        result = result + term.simple_div_with_scale(k, scale);

        k += 2;
        term = term * x_square;
        term.round_by_scale(scale);
    }
    result = result + arctan_02;
    result.round_by_scale(required_scale);
    return result;
}

// pi = 16 * arctan[1/5] - 4 * arctan[1/239]
BigDecimal pi(const size_t scale) {
    return BigDecimal("16") * arctan(BIG_DECIMAL_ZERO_TWO, scale)
            - BigDecimal("4") * arctan(BIG_DECIMAL_ONE.simple_div_with_scale(239, scale), scale);
}

// exp[x] = 1 + x + x^2/2 + x^3/6 + ...
BigDecimal exp(const BigDecimal &x, const size_t scale) {
    BigDecimal result = BIG_DECIMAL_ZERO;
    BigDecimal term = BIG_DECIMAL_ONE;
    uint64_t k = 1;
    while (!term.is_zero()) {
        result = result + term;
        term = (term * x).simple_div_with_scale(k, scale + kExtraScale);
        k++;
    }
    result.round_by_scale(scale);
    return result;
}

// ln[x] = 1 + ln[x / e]
// formula: ln[1 + x] = x - x^2/2 + x^3/3 - ...
BigDecimal ln(BigDecimal x, const size_t scale) {
    int64_t count = 0;
    BigDecimal e = exp(BIG_DECIMAL_ONE, scale + kExtraScale);
    while (x > BIG_DECIMAL_THREEHALFS) {
        x = x.div_with_scale(e, scale + kExtraScale);
        count++;
    }
    while (x < BIG_DECIMAL_HALF) {
        x = x * e;
        x.round_by_scale(scale + kExtraScale);
        count--;
    }

    BigDecimal result(to_string(count));
    BigDecimal term = x - BIG_DECIMAL_ONE;
    int64_t k = 1;
    x = BIG_DECIMAL_ONE - x;  // multiplier
    while (!term.is_zero()) {
        result = result + term.simple_div_with_scale(k, scale + kExtraScale);
        term = term * x;
        term.round_by_scale(scale + kExtraScale);
        k++;
    }
    result.round_by_scale(scale);
    return result;
}

BigDecimal phi(const BigDecimal &x, const size_t required_scale) {
    size_t scale = required_scale + kExtraScale;

    BigDecimal result = BIG_DECIMAL_ZERO;
    BigDecimal term = x;
    BigDecimal x_square = - x * x;
    uint64_t k = 1;
    while (!term.is_zero()) {
        result = result + term.simple_div_with_scale(2 * k - 1, scale);
        term = (term * x_square).simple_div_with_scale(2 * k, scale);
        k++;
    }

    BigDecimal coefficient_1 = BIG_DECIMAL_ONE.div_with_scale(sqrt(pi(scale).simple_mul(2, 0), scale), scale);

    BigDecimal ret = BIG_DECIMAL_HALF + coefficient_1 * result;
    ret.round_by_scale(required_scale);
    return ret;
}
