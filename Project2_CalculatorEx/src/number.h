#ifndef CALCULATOR_SRC_NUMBER_H
#define CALCULATOR_SRC_NUMBER_H

#include <cstdint>
#include <sstream>
#include <string>
#include <vector>

class BigDecimal;

class BigInteger {
 private:
    std::vector<uint8_t> digits_;  // one element is `kDigitWidth` digits

 public:
    BigInteger() : digits_() {}
    explicit BigInteger(std::vector<uint8_t> &&digits) : digits_(digits) { trim_leading_zeros(); }
    explicit BigInteger(std::string_view number);

    void trim_leading_zeros();  // standardize leading zero elements
    size_t trim_trailing_zeros();  // standardize trailing zero elements and return deleted number
    void add_one();  // add 1 to the BigInteger

    // get string representation of the integer, and the length of the string must be a multiple of 4
    // note: there may be several leading or trailing zeros, since it's expensive to standardize single zeros
    [[nodiscard]] std::string get_number_string() const;
    [[nodiscard]] const std::vector<uint8_t> &digits() const { return digits_; }
    [[nodiscard]] bool is_zero() const { return digits_.empty(); }

    [[nodiscard]] BigInteger left_shift(size_t length) const;  // returns *this * 10^length (i.e. left shift in base 10)
    BigInteger operator+(const BigInteger &other) const;
    BigInteger operator-(const BigInteger &other) const;
    BigInteger operator*(const BigInteger &other) const;
    BigInteger operator*(uint64_t rhs) const;
    BigInteger operator/(uint64_t rhs) const;

    bool operator<(const BigInteger &other) const;
    bool operator>(const BigInteger &rhs) const { return rhs < *this; }
    bool operator<=(const BigInteger &rhs) const { return !(rhs < *this); }
    bool operator>=(const BigInteger &rhs) const { return !(*this < rhs); }

    bool operator==(const BigInteger &rhs) const { return digits_ == rhs.digits_; }
    bool operator!=(const BigInteger &rhs) const { return !(rhs == *this); }

    friend class BigDecimal;
};

// represent a floating number using "m * 10^n", where m is `mantissa_`, and n is `exponent_`
class BigDecimal {
 private:
    BigInteger mantissa_;
    int64_t exponent_;
    bool positive_;

 public:
    explicit BigDecimal(BigInteger &&mantissa, int64_t exponent, bool positive)
            : mantissa_(mantissa), exponent_(exponent), positive_(positive) { standardize(); }
    explicit BigDecimal(std::string_view number);

    [[nodiscard]] const BigInteger &mantissa() const { return mantissa_; }
    [[nodiscard]] int64_t exponent() const { return exponent_; }
    [[nodiscard]] bool positive() const { return positive_; }
    [[nodiscard]] bool is_zero() const { return mantissa_.is_zero(); }
    [[nodiscard]] int64_t most_significant_exponent() const;

    void standardize();  // by standardization, every unique is mapped to a unique BigDecimal, make it easy to compare
    void round_by_significant(size_t length);  // round, so that len(mantissa_) <= length
    void round_by_scale(size_t scale);
    void round_to_integer() { round_by_scale(0); }
    void drop_decimal();  // only drop the decimal, no rounding. equivalent to "floor"

    BigDecimal operator-() const;
    BigDecimal operator+(const BigDecimal &other) const;
    BigDecimal operator-(const BigDecimal &other) const;
    BigDecimal operator*(const BigDecimal &other) const;
    BigDecimal operator/(const BigDecimal &other) const;  // not advise, use `div_with_scale` instead
    BigDecimal operator%(const BigDecimal &other) const;

    [[nodiscard]] BigDecimal simple_mul(uint64_t rhs, int64_t exponent) const;
    [[nodiscard]] BigDecimal div_with_scale(const BigDecimal &other, size_t scale) const;
    [[nodiscard]] BigDecimal simple_div_with_scale(uint64_t rhs, size_t scale) const;

    bool operator<(const BigDecimal &other) const;
    bool operator>(const BigDecimal &rhs) const { return rhs < *this; }
    bool operator<=(const BigDecimal &rhs) const { return !(rhs < *this); }
    bool operator>=(const BigDecimal &rhs) const { return !(*this < rhs); }
    bool operator==(const BigDecimal &rhs) const;
    bool operator!=(const BigDecimal &other) const { return !(*this == other); }

    friend std::ostream &operator<<(std::ostream &stream, const BigDecimal &decimal);
};

std::ostream &operator<<(std::ostream &stream, const BigDecimal &decimal);

#endif  // CALCULATOR_SRC_NUMBER_H
