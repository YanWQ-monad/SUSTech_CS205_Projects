#ifndef CALCULATOR_SRC_TOKEN_H
#define CALCULATOR_SRC_TOKEN_H

#include <cassert>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "number.h"

using Punctuator = char;
using Identifier = std::string;
using TokenContent = std::variant<BigDecimal, Identifier, Punctuator>;

struct TokenRange {
    size_t begin_, end_, frame_id_;

    static TokenRange single(const size_t pos, const size_t frame_id) {
        return TokenRange{pos, pos + 1, frame_id};
    }

    TokenRange operator+(const TokenRange &other) const {
        assert(frame_id_ == other.frame_id_);
        return TokenRange{begin_, other.end_, frame_id_};
    }

    bool operator==(const TokenRange &rhs) const {
        return begin_ == rhs.begin_ && end_ == rhs.end_ && frame_id_ == rhs.frame_id_;
    }

    bool operator!=(const TokenRange &rhs) const { return !(rhs == *this); }
};

class Token {
    TokenContent content_;
    TokenRange range_;

 public:
    Token(TokenContent content, const TokenRange &range) : content_(std::move(content)), range_(range) {}

    [[nodiscard]] TokenContent &content() { return content_; }
    [[nodiscard]] TokenRange &range() { return range_; }
};

std::vector<Token> tokenize(std::string_view input, size_t frame_id);

#endif  // CALCULATOR_SRC_TOKEN_H
