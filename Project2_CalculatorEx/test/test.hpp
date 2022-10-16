#ifndef CALCULATOR_TEST_H
#define CALCULATOR_TEST_H

#include <random>
#include <sstream>
#include <string>

#include "number.h"

static std::random_device rd;
static std::mt19937 rng{rd()};
static std::uniform_int_distribution<> digit_distrib('0', '9');

std::string remove_prefix(std::string s, char c) {
    s.erase(s.begin(), find_if(s.begin(), s.end(), [c](char ch) { return ch != c; }));
    return s;
}

std::string big_decimal_string(const BigDecimal &decimal) {
    std::ostringstream ss;
    ss << decimal;
    return ss.str();
}

static void generate_random_digits(std::string &s) {
    for (char &c : s)
        c = static_cast<char>(digit_distrib(rng));
    if (rng() & 1)
        s[0] = '-';
}

BigDecimal get_decimal(size_t size) {
    std::string s(size, '0');
    generate_random_digits(s);
    return BigDecimal(s);
}

#endif  // CALCULATOR_TEST_H
