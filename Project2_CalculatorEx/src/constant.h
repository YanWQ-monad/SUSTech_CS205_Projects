#ifndef CALCULATOR_SRC_CONSTANT_H
#define CALCULATOR_SRC_CONSTANT_H

#include "number.h"

constexpr size_t kExtraScale = 7;
constexpr int64_t kWarningDepth = 5000;
constexpr int64_t kDivergentLimit = 500000;

extern const BigDecimal BIG_DECIMAL_ZERO;        // 0
extern const BigDecimal BIG_DECIMAL_ZERO_TWO;    // 0.2
extern const BigDecimal BIG_DECIMAL_HALF;        // 0.5
extern const BigDecimal BIG_DECIMAL_ONE;         // 1
extern const BigDecimal BIG_DECIMAL_THREEHALFS;  // 1.5
extern const BigDecimal BIG_DECIMAL_TWO;         // 2

#endif  // CALCULATOR_SRC_CONSTANT_H
