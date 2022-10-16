#ifndef CALCULATOR_SRC_ERROR_H
#define CALCULATOR_SRC_ERROR_H

#include <exception>
#include <string>
#include <utility>
#include "token.h"

class application_error : public std::exception {
    using std::exception::exception;
};

class number_parse_error : public std::exception {
    const char *message_;

 public:
    explicit number_parse_error(const char *message) : message_(message) {}

    [[nodiscard]] const char *what() const noexcept override { return message_; }
};

class ranged_error : public application_error {
    TokenRange range_;
    std::string message_;

 public:
    ranged_error(const TokenRange &range, const std::string& message, const std::string& more)
            : range_(range), message_(message + ": " + more) {}

    ranged_error(const TokenRange &range, std::string message)
            : range_(range), message_(std::move(message)) {}

    [[nodiscard]] TokenRange range() const { return range_; }

    [[nodiscard]] const char *what() const noexcept override {
        return message_.c_str();
    }
};

class runtime_error : public application_error {
    std::string message_;

 public:
    explicit runtime_error(std::string message) : message_(std::move(message)) {}

    [[nodiscard]] const char *what() const noexcept override {
        return message_.c_str();
    }
};

class stackoverflow_warning : public ranged_error {
    using ranged_error::ranged_error;
};

class divergent_warning : public ranged_error {
    using ranged_error::ranged_error;
};

#endif  // CALCULATOR_SRC_ERROR_H
