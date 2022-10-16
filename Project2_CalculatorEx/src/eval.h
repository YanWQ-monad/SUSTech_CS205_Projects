#ifndef CALCULATOR_SRC_EVAL_H
#define CALCULATOR_SRC_EVAL_H

#include <functional>
#include "number.h"

BigDecimal newtons_method(const std::function<BigDecimal(const BigDecimal&)>& formula,
                          BigDecimal initial, size_t scale);

BigDecimal pow(BigDecimal x, BigDecimal y, size_t scale);
BigDecimal sqrt(const BigDecimal &x, size_t scale);
BigDecimal sin(const BigDecimal &x, size_t scale);
BigDecimal cos(const BigDecimal &x, size_t scale);
BigDecimal arctan(BigDecimal x, size_t scale);
BigDecimal pi(size_t scale);
BigDecimal exp(const BigDecimal &x, size_t scale);
BigDecimal ln(BigDecimal x, size_t scale);
BigDecimal phi(const BigDecimal &x, const size_t scale);

#endif  // CALCULATOR_SRC_EVAL_H
