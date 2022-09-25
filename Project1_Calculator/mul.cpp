#include <algorithm>
#include <cassert>
#include <complex>
#include <cstring>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

using std::cerr;
using std::complex;
using std::copy;
using std::cout;
using std::endl;
using std::exception;
using std::find;
using std::find_if;
using std::find_if_not;
using std::min;
using std::ostream;
using std::ostream_iterator;
using std::string;
using std::string_view;
using std::transform;
using std::vector;

class number_parse_error : exception {
 public:
    const char *reason_;

    explicit number_parse_error(const char *reason) : reason_(reason) {}

    [[nodiscard]] const char* what() const noexcept override {
        return "failed to parse the input as numbers";
    }
};

class FFTContext {
    vector<complex<double>> omega_, omega_inverse_;

 public:
    uint32_t n_, k_ = 0;  // n is the maximum size, and n = 1 << k

    // initialize an FFT context with maximum length `m`
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
            // reverse bits using Bit Twiddling Hacks
            // reference: https://graphics.stanford.edu/~seander/bithacks.html#ReverseParallel
            uint32_t t = i;
            t = ((t >> 1) & 0x55555555) | ((t & 0x55555555) << 1);
            t = ((t >> 2) & 0x33333333) | ((t & 0x33333333) << 2);
            t = ((t >> 4) & 0x0F0F0F0F) | ((t & 0x0F0F0F0F) << 4);
            t = ((t >> 8) & 0x00FF00FF) | ((t & 0x00FF00FF) << 8);
            t = ( t >> 16             ) | ( t               << 16);

            // general bits reverse is reverse on 32-bit, but we only want to reverse on k-bit,
            // so we can right shift (32 - k) bits to make things right
            t >>= (32 - k_);

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

class BigDecimal;  // declare here, so we can declare friend function inside BigInteger

class BigInteger {
 public:
    constexpr static int kDigitWidth = 4;
    constexpr static int kDigitRange = 10000;

 private:
    bool positive_;
    vector<uint16_t> digits_;  // one element is `kDigitWidth` digits

 public:
    BigInteger() : positive_(true), digits_() {}

    explicit BigInteger(string_view number) : positive_(true) {
        // check negative or positive
        if (!number.empty() && number[0] == '-') {
            positive_ = false;
            number.remove_prefix(1);
        }
        number.remove_prefix(min(number.size(), number.find_first_not_of('0')));  // remove leading zeros

        // check whether all digits are '0' to '9'
        if (find_if_not(number.begin(), number.end(),
                        [](char digit) { return '0' <= digit && digit <= '9'; }) != number.end()) {
            throw number_parse_error("not digit (0 to 9)");
        }

        // copy and transform digits to elements (`digits_`)
        //
        // `j` is how many digits should be in `digits_[0]`
        // consider a number:   "123 4567 8901", and `kDigitWidth` = 4
        //                       ---
        //                    j = 3, means that 3 digits "123" should be put in `digits_[0]`
        decltype(digits_)::value_type j = (number.length() - 1) % kDigitWidth + 1;
        digits_.reserve(number.length() / kDigitWidth + 1);
        size_t value = 0;  // temporary buffer
        for (char digit : number) {
            value = value * 10 + (digit - '0');
            if ((--j) == 0) {
                // if we have collected all the digits that one element of `digits_[ ]` needs, that is, `j == 0`,
                // we push it to `digits_` and reset `value`
                digits_.push_back(value);
                value = 0;
                j = kDigitWidth;  // reset counter `j`
            }
        }
        assert(value == 0 && j == kDigitWidth);  // assert all digits are exactly put in `digits_`
        assert(digits_.size() <= number.length() / kDigitWidth + 1);  // assert the served space is enough
    }

    // trim leading zero elements
    void trim_leading_zeros() {
        // find first non-zero digit
        auto non_zero = find_if_not(digits_.begin(), digits_.end(), [](auto digit) { return digit == 0; });
        digits_.erase(digits_.begin(), non_zero);  // then erase the leading zeros
    }

    // get string representation of the integer, and the length of the string must be a multiple of 4
    // note: there may be several leading or trailing zeros, since it's expensive to trim single zeros
    [[nodiscard]] string get_number_string() const {
        string s;
        s.reserve(digits_.size() * kDigitWidth);

        char buffer[kDigitWidth];
        for (auto element : digits_) {
            // first, serialize each element to a 4 digits string into `buffer`,
            // the most significant digit is at the end of `buffer`
            for (char &digit : buffer) {
                digit = static_cast<char>(element % 10 + '0');
                element /= 10;
            }

            // put content of `buffer` to `s` in reverse order (from the most significant digit to the least one)
            for (int i = kDigitWidth - 1; i >= 0; --i)
                s.push_back(buffer[i]);
        }

        assert(s.length() == digits_.size() * kDigitWidth);  // assert reserved space is enough
        return s;
    }

    BigInteger operator*(const BigInteger &other) const {
        BigInteger result;

        // simple formula to determinate whether it's positive, and can be easily proved by drawing a truth table
        result.positive_ = !positive_ ^ other.positive_;

        // prepare FFT context:
        // for two numbers with length `x` and `y`, the length of the multiplication result will be at most `x + y`
        FFTContext context(digits_.size() + other.digits_.size());
        vector<complex<double>> lhs(context.n_), rhs(context.n_);

        // copy the digits to FFT coefficients:
        // when we store digits, we place the digits to array from the most significant digit (in `digits_[0]`) to the
        // least significant digit. but when we perform FFT, the coefficients should be placed from the least
        // significant digit (in `digits_[0]`) to the most significant digit. so we need to copy `digits_` to
        // `vector<complex<double>>` in reverse order
        copy(digits_.rbegin(), digits_.rend(), lhs.begin());
        copy(other.digits_.rbegin(), other.digits_.rend(), rhs.begin());

        // multiply via FFT:
        // first we transform the polynomial from coefficient representation to point-value representation by DFT
        context.dft(lhs);
        context.dft(rhs);
        // then we perform multiplication on the point values, "lhs <- lhs * rhs"
        transform(lhs.begin(), lhs.end(), rhs.begin(), lhs.begin(), [](auto x, auto y) { return x * y; });
        // next we transform the polynomial from point-value representation back to coefficient representation by I-DFT
        context.inverse_dft(lhs);

        // collect results from the polynomial, or you can just think that substituting x = 10 into the polynomial to
        // calculate the value
        result.digits_.reserve(context.n_);
        int64_t carry = 0;
        for (const auto &p : lhs) {
            carry += static_cast<decltype(carry)>(round(p.real()));
            result.digits_.push_back(static_cast<decltype(digits_[0])>(carry % kDigitRange));
            carry /= kDigitRange;
        }

        // reverse the digits, the reason is same as why copying in reverse order above
        reverse(result.digits_.begin(), result.digits_.end());

        result.trim_leading_zeros();  // standardization
        return result;
    }

    [[nodiscard]] bool is_positive() const {
        return positive_;
    }

    friend ostream &operator<<(ostream &stream, const BigDecimal &decimal);
};

class BigDecimal {
 private:
    // represent a floating number using "m * 10^n", where m is `mantissa_`, and n is `exponent_`
    BigInteger mantissa_;
    int64_t exponent_;

 public:
    explicit BigDecimal(BigInteger &&mantissa, int64_t exponent) : mantissa_(mantissa), exponent_(exponent) {}

    explicit BigDecimal(string_view number) {
        if (!number.empty() && number[0] == '+')
            number.remove_prefix(1);  // remove leading "+"

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
            char *str_end;
            errno = 0;  // reset errno
            exponent_ = strtoll(number.substr(e_pos + 1).data(), &str_end, 10);
            if (errno == ERANGE)  // explicit error occurred
                throw number_parse_error("exponent out of range");
            if (str_end != number.end())  // did not parse the whole string (stopped in the middle)
                throw number_parse_error("invalid exponent");
        }
        exponent_ -= static_cast<int64_t>(part2.length());  // do not forget the decimal part in part2
    }

    BigDecimal operator*(const BigDecimal &other) const {
        // multiply mantissas and add exponents
        BigInteger mantissa = mantissa_ * other.mantissa_;
        int64_t exponent = exponent_ + other.exponent_;
        return BigDecimal(std::move(mantissa), exponent);
    }

    friend ostream &operator<<(ostream &stream, const BigDecimal &decimal);
};

ostream &operator<<(ostream &stream, const BigDecimal &decimal) {
    // special condition for 0
    if (decimal.mantissa_.digits_.empty())
        return stream << '0';

    // serialize `mantissa` to string, then make it into `string_view`, so that we can easily cut the slice
    string mantissa_part = decimal.mantissa_.get_number_string();
    string_view mantissa_view = mantissa_part;

    // assert that it's at least one non-zero digits, that is, return value of `find_first_not_of` won't be `npos`
    // because every `BigDecimal` instance is well trimmed, so if all the digits are zero, then it will be trimmed to
    // empty (`digits_` is empty), and this case is already handled by the if on the head of this function,
    // so here there are at least one non-zero digits.
    assert(mantissa_view.find_first_not_of('0') != string::npos);
    // trim leading zeros of `mantissa_view`
    mantissa_view.remove_prefix(mantissa_view.find_first_not_of('0'));

    // how many digits should be print before decimal point '.'
    //
    // consider mantissa is "12345", there are some cases for `integer_length`:
    // 1. when `integer_length` =  2, then the result is "12.345", a simple case
    // 2. when `integer_length` =  8 > 5, it means "[12345]000", note that we need to fill trailing zeros
    // 3. when `integer_length` = -2 < 0, it means "0.00[12345]", note that we need to fill zeros after '.'
    int64_t integer_length = static_cast<int64_t>(mantissa_view.length()) + decimal.exponent_;

    // trim trailing zeros of `mantissa_view`, note that it should be done after `integer_length`
    assert(mantissa_view.find_last_not_of('0') != string::npos);  // assert it's at least one non-zero digits, too
    mantissa_view.remove_suffix(mantissa_view.size() - mantissa_view.find_last_not_of('0') - 1);

    // negative sign
    if (!decimal.mantissa_.positive_)
        stream << '-';

    // if so, print in scientific notation
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
        if (integer_length <= 0) {  // if integer part is zero, then print one '0'
            stream << '0';
        } else {
            // determinate how many characters in `mantissa_view` need to print
            int64_t view_length = min(integer_length, static_cast<int64_t>(mantissa_view.length()));

            // print mantissa to integer part
            stream << mantissa_view.substr(0, view_length);
            mantissa_view.remove_prefix(view_length);
            integer_length -= view_length;

            // fill trailing zeros, if `mantissa_` is not enough (case 2)
            fill_n(ostream_iterator<char>(stream), integer_length, '0');
        }

        // if `mantissa_view` is not empty, we need to print decimal part
        if (!mantissa_view.empty()) {
            stream << '.';

            // if `integer_length < 0`, we need to fill zeros after '.' before printing mantissa (case 3)
            fill_n(ostream_iterator<char>(stream), -integer_length, '0');

            stream << mantissa_view;
        }
    }

    return stream;
}

struct options {
    bool scientific = false;
    int64_t scientific_precision = -1;
};

void print_help(const char *executable) {
    cout << "USAGE: " << executable << " <A> <B> [options...]" << endl;
    cout << R"(
ARGUMENTS:
  <A> <B>                 The multiplier and multiplicand
                          You can use either fixed number (i.e. 10.17) or scientific notation (i.e. 1.017e+01)

OPTIONS:
  -s, --scientific [N]    Print in scientific notation. If N is supplied, the precision of mantissa will be set to N
)";
}

options parse_options(int argc, char *argv[]) {
    options option;

    if (argc < 3) {
        if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
            print_help(argv[0]);
            exit(0);
        } else {
            cerr << "You input less numbers than expected, use \"--help\" for help" << endl;
            exit(1);
        }
    }

    // skip program name and A, B, starts with 3
    for (int i = 3; i < argc; i++) {
        if (!strcmp("-s", argv[i]) || !strcmp("--scientific", argv[i])) {
            option.scientific = true;

            // check the following optional N ("--scientific [N]")
            if (i + 1 < argc) {
                try {
                    option.scientific_precision = std::stoll(argv[i + 1]);
                    i++;
                }
                catch (std::logic_error &) {}
            }
            continue;
        }

        cerr << "Unrecognized option: " << argv[i] << endl;
        cerr << "Maybe you input more numbers than expected" << endl;
        exit(1);
    }

    return option;
}

int main(int argc, char *argv[]) {
    // since we do not use C-like IO function, we can safely disable the sync
    std::ios_base::sync_with_stdio(false);

    options option = parse_options(argc, argv);
    if (option.scientific) {
        cout << std::scientific;
        if (option.scientific_precision != -1)
            cout.precision(option.scientific_precision);
    }

    try {
        // parse number from `argv[1]` and `argv[2]` respectively, then multiply them
        BigDecimal multiplier(argv[1]), multiplicand(argv[2]);
        BigDecimal result = multiplier * multiplicand;

        cout << multiplier << " * " << multiplicand << " = " << result << endl;
    } catch (number_parse_error &e) {
        cerr << "The input cannot be interpreted as numbers: " << e.reason_ << endl;
        return 1;
    } catch (exception &e) {
        cerr << "Unknown exception: " << e.what() << endl;
        return 1;
    }

    return 0;
}
