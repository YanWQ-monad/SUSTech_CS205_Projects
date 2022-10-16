#ifndef CALCULATOR_SRC_PARSE_H
#define CALCULATOR_SRC_PARSE_H

#include <memory>
#include <string>
#include <utility>

#include "node.h"

using ExpressionStm = std::unique_ptr<Expression>;
ExpressionStm parse(std::string_view input, size_t frame_id);

#endif  // CALCULATOR_SRC_PARSE_H
