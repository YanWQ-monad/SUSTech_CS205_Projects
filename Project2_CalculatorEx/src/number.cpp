#include <algorithm>
#include <complex>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>
#include <utility>

#include "constant.h"
#include "error.h"
#include "eval.h"
#include "number.h"

using std::back_inserter;
using std::complex;
using std::copy;
using std::find;
using std::find_if;
using std::find_if_not;
using std::lexicographical_compare;
using std::max;
using std::min;
using std::ostream;
using std::ostream_iterator;
using std::reverse;
using std::string;
using std::string_view;
using std::transform;
using std::vector;

inline uint32_t uint32_bit_reverse(uint32_t t) {
    // reverse bits using Bit Twiddling Hacks
    // reference: https://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
    t = ((t >> 1) & 0x55555555) | ((t & 0x55555555) << 1);
    t = ((t >> 2) & 0x33333333) | ((t & 0x33333333) << 2);
    t = ((t >> 4) & 0x0F0F0F0F) | ((t & 0x0F0F0F0F) << 4);
    t = ((t >> 8) & 0x00FF00FF) | ((t & 0x00FF00FF) << 8);
    t = (t >> 16) | (t << 16);
    return t;
}

class FFTContext {
    vector<complex<double>> omega_, omega_inverse_;

 public:
    uint32_t n_, k_ = 0;  // n is the maximum size, and n = 1 << k

    // initialize an FFT context with minimum length `m`
    explicit FFTContext(const uint32_t m) {
        while ((1U << k_) < m)
            ++k_;
        n_ = 1U << k_;

        omega_.reserve(n_ >> 1);
        omega_inverse_.reserve(n_ >> 1);
        for (uint32_t i = 0; i < (n_ >> 1); ++i)
            omega_.emplace_back(cos(2 * M_PI / n_ * i), sin(2 * M_PI / n_ * i));
        ::transform(omega_.begin(), omega_.end(), back_inserter(omega_inverse_),
                  [](auto p) { return conj(p); });
    }

    void transform(vector<complex<double>> &a, const vector<complex<double>> &omega) const {
        assert(a.size() == n_);

        for (uint32_t i = 0; i < n_; ++i) {
            // general bits reverse is reverse on 32-bit, but we only want to reverse on k-bit,
            // so we can right shift (32 - k) bits to make things right
            uint32_t t = uint32_bit_reverse(i) >> (32 - k_);

            if (i < t)
                swap(a[i], a[t]);
        }

        for (uint32_t i = 1; i <= k_; ++i) {
            uint32_t omega_step = 1U << (k_ - i);
            for (auto p = a.begin(); p != a.end(); p += 1U << i) {
                auto l = p, r = p + (1U << (i - 1));
                for (auto omega_iter = omega.begin(); omega_iter != omega.end(); ++l, ++r, omega_iter += omega_step) {
                    complex<double> t = (*omega_iter) * (*r);
                    *r = *l - t;
                    *l += t;
                }
            }
        }
    }

    void dft(vector<complex<double>> &a) const {
        transform(a, omega_);
    }

    void inverse_dft(vector<complex<double>> &a) const {
        transform(a, omega_inverse_);
        for (auto &p : a)
            p /= n_;
    }
};

BigInteger::BigInteger(string_view number) {
    number.remove_prefix(min(number.size(), number.find_first_not_of('0')));  // remove leading zeros

    // check whether all digits are '0' to '9'
    if (find_if_not(number.begin(), number.end(),
                    [](char digit) { return '0' <= digit && digit <= '9'; }) != number.end()) {
        throw number_parse_error("not digit (0 to 9)");
    }

    // copy and transform digits to elements (`digits_`)
    digits_.reserve(number.length());
    transform(number.rbegin(), number.rend(), back_inserter(digits_), [](char digit) { return digit - '0'; });
}

void BigInteger::trim_leading_zeros() {
    // find first non-zero digit (from end() to begin())
    auto non_zero = find_if_not(digits_.rbegin(), digits_.rend(), [](auto digit) { return digit == 0; });
    digits_.erase(digits_.end() - (non_zero - digits_.rbegin()), digits_.end());
}

size_t BigInteger::trim_trailing_zeros() {
    // find last non-zero digit
    auto non_zero = find_if_not(digits_.begin(), digits_.end(), [](auto digit) { return digit == 0; });
    size_t count = non_zero - digits_.begin();
    digits_.erase(digits_.begin(), non_zero);  // then erase the leading zeros
    return count;
}

void BigInteger::add_one() {
    int carry = 1;
    for (auto &x : digits_) {
        x += carry;
        carry = x >= 10;
        x -= carry ? 10 : 0;
    }
    if (carry > 0)
        digits_.push_back(carry);
}

string BigInteger::get_number_string() const {
    string s;

    // reserve the space then transform in reversed order
    s.reserve(digits_.size());
    transform(digits_.rbegin(), digits_.rend(), back_inserter(s), [](auto element) { return element + '0'; });

    return s;
}

BigInteger BigInteger::left_shift(size_t length) const {
    auto copy = *this;
    if (!digits_.empty())
        copy.digits_.insert(copy.digits_.begin(), length, 0);
    return copy;
}

BigInteger BigInteger::operator+(const BigInteger &other) const {
    BigInteger result;
    result.digits_.resize(max(digits_.size(), other.digits_.size()) + 1);

    // first, just put them together without handling the carry
    copy(digits_.begin(), digits_.end(), result.digits_.begin());
    transform(other.digits_.begin(), other.digits_.end(), result.digits_.begin(), result.digits_.begin(),
              [](auto x, auto y) { return x + y; });

    // then handle the carry by scanning the digits
    int carry = 0;
    for (auto &x : result.digits_) {
        x += carry;
        carry = x >= 10;
        x -= carry ? 10 : 0;
    }
    return result;
}

BigInteger BigInteger::operator-(const BigInteger &other) const {
    BigInteger result;
    result.digits_.resize(max(digits_.size(), other.digits_.size()));

    // same as +, first put together without carry
    copy(digits_.begin(), digits_.end(), result.digits_.begin());
    transform(other.digits_.begin(), other.digits_.end(), result.digits_.begin(), result.digits_.begin(),
              [](auto x, auto y) { return y - x; });

    int carry = 0;
    for (auto &x : result.digits_) {
        x -= carry;
        carry = x > 10;  // since `digits_` is unsigned, so negative number will underflow to a very large number
        x += carry ? 10 : 0;
    }
    return result;
}

BigInteger BigInteger::operator*(const BigInteger &other) const {
    BigInteger result;

    // prepare FFT context:
    // for two numbers with length `x` and `y`, the length of the multiplication result will be at most `x + y`
    FFTContext context(digits_.size() + other.digits_.size());
    vector <complex<double>> lhs(context.n_), rhs(context.n_);

    // copy the digits to FFT coefficients:
    copy(digits_.begin(), digits_.end(), lhs.begin());
    copy(other.digits_.begin(), other.digits_.end(), rhs.begin());

    // multiply via FFT:
    // first we transform the polynomial from coefficient representation to point-value representation by DFT
    context.dft(lhs);
    context.dft(rhs);
    // then we perform multiplication on the point values, "lhs_ <- lhs_ * rhs_"
    transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), [](auto x, auto y) { return x * y; });
    // next we transform the polynomial from point-value representation back to coefficient representation by I-DFT
    context.inverse_dft(lhs);

    // collect results from the polynomial, or you can just think that substituting x = 10 into the polynomial to
    // calculate the value
    result.digits_.reserve(context.n_);
    int64_t carry = 0;
    for (const auto &p : lhs) {
        carry += static_cast<decltype(carry)>(round(p.real()));
        result.digits_.push_back(static_cast<decltype(digits_[0])>(carry % 10));
        carry /= 10;
    }

    result.trim_leading_zeros();  // standardization
    return result;
}

// simple multiplication with integer rhs
BigInteger BigInteger::operator*(const uint64_t rhs) const {
    BigInteger result = *this;
    uint64_t carry = 0;

    // apply the multiplier to all the digits
    for (auto &x : result.digits_) {
        carry += x * rhs;
        x = carry % 10;
        carry /= 10;
    }

    // if carry is not zero, we need to "add" some digits
    while (carry > 0) {
        result.digits_.push_back(carry % 10);
        carry /= 10;
    }

    return result;
}

// simple division with integer rhs
BigInteger BigInteger::operator/(const uint64_t rhs) const {
    BigInteger result = *this;

    uint64_t dividend = 0;
    for (auto iter = result.digits_.rbegin(); iter != result.digits_.rend(); ++iter) {
        dividend = dividend * 10 + (*iter);
        *iter = dividend / rhs;
        dividend %= rhs;
    }

    // round
    if (dividend * 2 >= rhs)
        result.add_one();

    result.trim_leading_zeros();
    return result;
}

bool BigInteger::operator<(const BigInteger &other) const {
    if (digits_.size() != other.digits_.size())
        return digits_.size() < other.digits_.size();
    return lexicographical_compare(digits_.rbegin(), digits_.rend(), other.digits_.rbegin(), other.digits_.rend());
}

BigDecimal::BigDecimal(string_view number) : positive_(true) {
    assert(!number.empty());  // guarantee by the caller
    switch (number[0]) {
        case '-':
            positive_ = false;
            [[fallthrough]];
        case '+':
            number.remove_prefix(1);
    }

    // try to parse as scientific notation, which also includes simple floating number and simple integer
    // expected format:   ["-"] <digits> ["." <digits>] ["e" ["-"] <digits>]
    //                    --------------   ^  --------    ^  --------------
    //                        part1        |    part2     |       part3
    //                                  dot_pos         e_pos
    //
    // if it's integer, then part2 and part3 is none
    // if it's simple floating number, then part3 is none
    // if it's scientific notation, none is none

    // first, we find the position of 'e'. and since 'e' usually appears in the right of the number (if it exists),
    // searching from right to left will lead to a better performance.
    auto riter = find_if(number.rbegin(), number.rend(), [](char c) { return c == 'e' || c == 'E'; });
    // translate `reverse_iterator` to common `iterator`
    // if 'e' not found, `e_pos_iter` will be `number.end()`, otherwise the position.
    auto e_pos_iter = riter == number.rend() ? number.end() : number.end() - (riter - number.rbegin()) - 1;

    size_t e_pos = e_pos_iter - number.begin();
    // find in [begin, e_pos_iter) to make sure `dot_pos < e_pos`
    size_t dot_pos = find(number.begin(), e_pos_iter, '.') - number.begin();

    string_view part1 = number.substr(0, dot_pos);
    string_view part2 = e_pos == dot_pos ? "" : number.substr(dot_pos + 1, e_pos - dot_pos - 1);

    // parse mantissa
    string mantissa_part;
    mantissa_part.reserve(part1.length() + part2.length());
    mantissa_part.append(part1).append(part2);  // skip '.' and parse mantissa part
    mantissa_ = BigInteger(mantissa_part);

    // parse exponent
    if (e_pos_iter == number.end()) {  // if 'e' not found
        exponent_ = 0;
    } else {
        try {
            size_t pos;
            exponent_ = std::stoll(string(number.substr(e_pos + 1)), &pos);

            // if it didn't parse the whole string
            if (pos != number.substr(e_pos + 1).length())
                throw std::invalid_argument("");
        } catch (std::out_of_range &) {
            throw number_parse_error("exponent out of range");
        } catch (std::invalid_argument &) {
            throw number_parse_error("invalid exponent");
        }
    }
    exponent_ -= static_cast<int64_t>(part2.length());  // do not forget the decimal part in part2

    standardize();
}

int64_t BigDecimal::most_significant_exponent() const {
    if (is_zero())
        return std::numeric_limits<int64_t>::min();  // 0 => -INF

    return mantissa_.digits_.size() + exponent_;
}

void BigDecimal::standardize() {
    mantissa_.trim_leading_zeros();
    exponent_ += mantissa_.trim_trailing_zeros();

    if (is_zero()) {
        positive_ = true;
        exponent_ = 0;
    }
}

void BigDecimal::round_by_significant(size_t length) {
    if (mantissa_.digits_.size() <= length)
        return;

    // update the exponent
    exponent_ += mantissa_.digits_.size() - length;

    // save it so we can round it later
    auto last_digit = *(mantissa_.digits_.end() - length - 1);

    // remove redundant digits
    mantissa_.digits_.erase(mantissa_.digits_.begin(), mantissa_.digits_.end() - length);

    if (last_digit >= 5)  // round
        mantissa_.add_one();

    // if `add_one` adds a digit, then we need to erase one more digit
    if (mantissa_.digits_.size() > length) {
        auto last_digit_2 = mantissa_.digits_[0];
        mantissa_.digits_.erase(mantissa_.digits_.begin());
        if (last_digit_2 >= 5)  // round
            mantissa_.add_one();  // by simple analysis, this won't add a digit
        exponent_ += 1;
    }

    assert(mantissa_.digits_.size() <= length);
    standardize();
}

void BigDecimal::round_by_scale(size_t scale) {
    if (- exponent_ <= static_cast<int64_t>(scale))
        return;

    // if the most significant exponent is smaller than -scale
    // that means it will be trim to zero
    // i.e. round(0.01, 1) => 0
    if (- most_significant_exponent() > static_cast<int64_t>(scale))
        round_by_significant(0);
    else  // else significant digits = integer part + decimal part
        round_by_significant(most_significant_exponent() + scale);
}

void BigDecimal::drop_decimal() {
    if (exponent_ >= 0)
        return;

    mantissa_.digits_.erase(mantissa_.digits_.begin(), mantissa_.digits_.begin() + (-exponent_));
    exponent_ = 0;
    standardize();
}

BigDecimal BigDecimal::operator-() const {
    auto copy = *this;
    copy.positive_ = !copy.positive_;
    return copy;
}

BigDecimal BigDecimal::operator+(const BigDecimal &other) const {
    if (!(exponent_ <= other.exponent_))
        return other + *this;
    // after that, we can guarantee `exponent_ <= other.exponent_` now

    // make `mantissa_` and `that` under the same exponent
    BigInteger that = other.mantissa_.left_shift(other.exponent_ - exponent_);
    if (positive_ == other.positive_)
        return BigDecimal(mantissa_ + that, exponent_, positive_);
    else if (mantissa_ >= that)
        return BigDecimal(mantissa_ - that, exponent_, positive_);
    else
        return BigDecimal(that - mantissa_, exponent_, other.positive_);
}

BigDecimal BigDecimal::operator-(const BigDecimal &other) const {
    if (!(exponent_ <= other.exponent_)) {
        BigDecimal result = other - *this;
        // using `-result` will make an unnecessary copy, we can avoid it
        result.positive_ = !result.positive_;
        return result;
    }
    // after that, we can guarantee `exponent_ <= other.exponent_` now

    BigInteger that = other.mantissa_.left_shift(other.exponent_ - exponent_);
    if (positive_ != other.positive_)
        return BigDecimal(mantissa_ + that, exponent_, positive_);
    else if (mantissa_ >= that)
        return BigDecimal(mantissa_ - that, exponent_, positive_);
    else
        return BigDecimal(that - mantissa_, exponent_, !positive_);
}

BigDecimal BigDecimal::operator*(const BigDecimal &other) const {
    // multiply mantissas and add exponents
    BigInteger mantissa = mantissa_ * other.mantissa_;
    int64_t exponent = exponent_ + other.exponent_;
    bool positive = !(positive_ ^ other.positive_);
    return BigDecimal(std::move(mantissa), exponent, positive);
}

BigDecimal BigDecimal::operator/(const BigDecimal &other) const {
    return div_with_scale(other, 20);
}

BigDecimal BigDecimal::operator%(const BigDecimal &other) const {
    // we do not need too high scale, since the decimal part will be dropped later
    BigDecimal div = div_with_scale(other, kExtraScale);
    div.drop_decimal();
    return *this - div * other;
}

BigDecimal BigDecimal::simple_mul(const uint64_t rhs, const int64_t exponent) const {
    BigInteger mantissa = mantissa_ * rhs;
    return BigDecimal(std::move(mantissa), exponent_ + exponent, positive_);
}

/*
int64_t extract_high_k_digit(const BigInteger &integer, const size_t k) {
    int64_t result = 0;
    size_t length = integer.digits().size();
    for (size_t i = 1; i <= k; i++) {
        result *= 10;
        result += integer.digits()[length - i];
    }
    return result;
}

BigDecimal BigDecimal::div_with_scale(const BigDecimal &other, size_t scale) const {
    if (other.is_zero())
        throw runtime_error("div by zero");

    BigDecimal lhs = *this;
    BigDecimal rhs = other;

    bool positive = !(lhs.positive_ ^ rhs.positive_);
    lhs.positive_ = true;
    rhs.positive_ = true;

    BigDecimal rhs_result = BIG_DECIMAL_ONE;
    while (lhs.most_significant_exponent() > rhs.most_significant_exponent()) {
        rhs.exponent_ ++;
        rhs_result.exponent_ ++;
    }

    const int64_t rhs_est = extract_high_k_digit(rhs.mantissa_, 5);
    if (rhs_est == 141) {
        int i = 0;
    }
    BigDecimal result = BIG_DECIMAL_ZERO;
    while (rhs_result.most_significant_exponent() >= - static_cast<int64_t>(scale + kExtraScale)) {
        rhs.exponent_ --;
        rhs_result.exponent_ --;

        if (rhs.most_significant_exponent() >= lhs.most_significant_exponent())
            continue;

        const int64_t lhs_est = extract_high_k_digit(lhs.mantissa_, 6);
        const int64_t est = lhs_est / rhs_est;
        lhs = lhs - rhs.simple_mul(est, 0);
        result = result + rhs_result.simple_mul(est, 0);

        if (!lhs.positive_) {
            lhs = lhs + rhs;
            result = result - rhs_result;
        }
    }

    result.positive_ = positive;
    result.round_by_scale(scale);

    return result;
}
*/

// we first calculate `1/rhs` using Newton's Method
//    calculate by resolving f(x) = 1/x - rhs = 0
//    Newton's iteration: x <= (2 - x * rhs) * x (with a good initial)
// then use it to calculate `*this * (1/rhs)` to get the result
BigDecimal BigDecimal::div_with_scale(const BigDecimal &rhs, const size_t scale) const {
    if (rhs.mantissa_.is_zero())
        throw runtime_error("div by zero");

    size_t div_scale = max(static_cast<int64_t>(0), most_significant_exponent() + static_cast<int64_t>(scale + kExtraScale));

    // as rhs = "m * 10^n", use "1 * 10^{-n}" as initial
    BigDecimal initial(BigInteger({1}), - rhs.most_significant_exponent(), rhs.positive_);

    BigDecimal inv = newtons_method([&rhs](auto &x) { return (BIG_DECIMAL_TWO - x * rhs) * x; },
                                    std::move(initial), div_scale);
    BigDecimal result = *this * inv;
    result.round_by_scale(scale);
    return result;
}

// simple division with integer rhs and scale
BigDecimal BigDecimal::simple_div_with_scale(const uint64_t rhs, const size_t scale) const {
    if (rhs == 0)
        throw runtime_error("div by zero");

    // if the size of `mantissa_` can not fit `scale`, then extend it
    BigInteger lhs = mantissa_;
    int64_t exponent = exponent_;
    if (- exponent_ < static_cast<int64_t>(scale)) {
        lhs = mantissa_.left_shift(scale + exponent_);
        exponent = - static_cast<int64_t>(scale);
    } else {
        lhs = mantissa_;
    }

    BigDecimal result(lhs / rhs, exponent, positive_);
    result.round_by_scale(scale);
    return result;
}

bool BigDecimal::operator<(const BigDecimal &other) const {
    if (positive_ != other.positive_)
        return positive_ < other.positive_;
    if (most_significant_exponent() != other.most_significant_exponent())
        return most_significant_exponent() < other.most_significant_exponent();
    return lexicographical_compare(mantissa_.digits_.rbegin(), mantissa_.digits_.rend(),
                                   other.mantissa_.digits_.rbegin(), other.mantissa_.digits_.rend());
}

bool BigDecimal::operator==(const BigDecimal &rhs) const {
    return mantissa_ == rhs.mantissa_ &&
           exponent_ == rhs.exponent_ &&
           positive_ == rhs.positive_;
}

ostream &operator<<(ostream &stream, const BigDecimal &decimal) {
    // special condition for 0
    if (decimal.mantissa_.digits().empty())
        return stream << '0';

    // serialize `mantissa` to string, then make it into `string_view`, so that we can easily cut the slice
    string mantissa_part = decimal.mantissa_.get_number_string();
    string_view mantissa_view = mantissa_part;

    // assert that it's at least one non-zero digits, that is, return value of `find_first_not_of` won't be `npos`
    // because every `BigDecimal` instance is well trimmed, so if all the digits are zero, then it will be trimmed to
    // empty (`digits_` is empty), and this case is already handled by the if on the head of this function,
    // so here there are at least one non-zero digits.
    assert(mantissa_view.find_first_not_of('0') != string::npos);
    // standardize leading zeros of `mantissa_view`
    mantissa_view.remove_prefix(mantissa_view.find_first_not_of('0'));

    // how many digits should be debug_print before decimal point '.'
    //
    // consider mantissa is "12345", there are some cases for `integer_length`:
    // 1. when `integer_length` =  2, then the result is "12.345", a simple case
    // 2. when `integer_length` =  8 > 5, it means "[12345]000", note that we need to fill trailing zeros
    // 3. when `integer_length` = -2 < 0, it means "0.00[12345]", note that we need to fill zeros after '.'
    int64_t integer_length = static_cast<int64_t>(mantissa_view.length()) + decimal.exponent_;

    // standardize trailing zeros of `mantissa_view`, note that it should be done after `integer_length`
    assert(mantissa_view.find_last_not_of('0') != string::npos);  // assert it's at least one non-zero digits, too
    mantissa_view.remove_suffix(mantissa_view.size() - mantissa_view.find_last_not_of('0') - 1);

    // negative sign
    if (!decimal.positive_)
        stream << '-';

    // if so, debug_print in scientific notation
    if (stream.flags() & std::ios_base::scientific) {
        int64_t precision = stream.precision();
        int64_t output_exponent = integer_length - 1;  // integer part has only 1 digit, so subtract 1

        stream << mantissa_view[0];  // integer part

        // skip if precision is zero or no more digits need to be printed
        if (precision > 0 && mantissa_view.length() > 1)
            stream << '.' << mantissa_view.substr(1, precision);  // `precision` too big won't cause overflow
        char exponent_sign = output_exponent >= 0 ? '+' : '-';
        stream << 'e' << exponent_sign << abs(output_exponent);
    } else {
        if (integer_length <= 0) {  // if integer part is zero, then debug_print one '0'
            stream << '0';
        } else {
            // determinate how many characters in `mantissa_view` need to debug_print
            int64_t view_length = min(integer_length, static_cast<int64_t>(mantissa_view.length()));

            // debug_print mantissa to integer part
            stream << mantissa_view.substr(0, view_length);
            mantissa_view.remove_prefix(view_length);
            integer_length -= view_length;

            // fill trailing zeros, if `mantissa_` is not enough (case 2)
            fill_n(ostream_iterator<char>(stream), integer_length, '0');
        }

        // if `mantissa_view` is not empty, we need to debug_print decimal part
        if (!mantissa_view.empty()) {
            stream << '.';

            // if `integer_length < 0`, we need to fill zeros after '.' before printing mantissa (case 3)
            fill_n(ostream_iterator<char>(stream), -integer_length, '0');

            stream << mantissa_view;
        }
    }

    return stream;
}
