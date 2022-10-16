#ifndef CALCULATOR_SRC_NODE_H
#define CALCULATOR_SRC_NODE_H

#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "number.h"
#include "token.h"

class Context;

class Expression {
 protected:
    TokenRange range_;

 public:
    explicit Expression(const TokenRange &range) : range_(range) {}
    virtual ~Expression() = default;

    [[nodiscard]] const TokenRange &range() const { return range_; }
    [[nodiscard]] TokenRange &range() { return range_; }

    virtual BigDecimal eval(Context &context) = 0;
    virtual void print(std::ostream &stream) const = 0;
};

class NumericNode : public Expression {
    BigDecimal number_;

 public:
    explicit NumericNode(BigDecimal &&number, TokenRange range) : Expression(range), number_(number) {}

    BigDecimal eval([[maybe_unused]] Context &context) override { return number_; }
    void print(std::ostream &stream) const override;
};

class VariableNode : public Expression {
    std::string name_;

 public:
    explicit VariableNode(std::string name, TokenRange range) : Expression(range), name_(std::move(name)) {}

    [[nodiscard]] const std::string& name() const { return name_; }

    BigDecimal eval(Context &context) override;
    void print(std::ostream &stream) const override;
};

class BinOpNode : public Expression {
 public:
    enum BinaryOperationType {
        BinOp_ADD = '+',
        BinOp_SUB = '-',
        BinOp_MUL = '*',
        BinOp_DIV = '/',
        BinOp_MOD = '%',
        BinOp_LE = '<',
        BinOp_GE = '>',
    };

 private:
    std::unique_ptr<Expression> lhs_, rhs_;
    BinaryOperationType type_;

 public:
    BinOpNode(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs,
              BinaryOperationType type, TokenRange range)
            : Expression(range), lhs_(std::move(lhs)), rhs_(std::move(rhs)), type_(type) {}

    BigDecimal eval_wrapper(Context &context);
    BigDecimal eval(Context &context) override;
    void print(std::ostream &stream) const override;
};

class FunctionNode : public Expression {
    std::string name_;
    std::vector<std::unique_ptr<Expression>> args_;

 public:
    FunctionNode(std::string name, std::vector<std::unique_ptr<Expression>> args, TokenRange range)
            : Expression(range), name_(std::move(name)), args_(std::move(args)) {}

    [[nodiscard]] const std::string &name() const { return name_; }
    [[nodiscard]] const std::vector<std::unique_ptr<Expression>> &args() const { return args_; }

    BigDecimal eval(Context &context) override;
    void print(std::ostream &stream) const override;
};

class SequenceNode : public Expression {
    std::unique_ptr<Expression> lhs_, rhs_;

 public:
    SequenceNode(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs, TokenRange range)
            : Expression(range), lhs_(std::move(lhs)), rhs_(std::move(rhs)) {}

    BigDecimal eval(Context &context) override;
    void print(std::ostream &stream) const override;
};

class AssignmentNode : public Expression {
    std::string name_;
    std::unique_ptr<Expression> expression_;

 public:
    AssignmentNode(std::string name, std::unique_ptr<Expression> expression, TokenRange range)
            : Expression(range), name_(std::move(name)), expression_(std::move(expression)) {}

    [[nodiscard]] const std::string &name() const { return name_; }
    [[nodiscard]] const std::unique_ptr<Expression> &expression() const { return expression_; }

    BigDecimal eval(Context &context) override;
    void print(std::ostream &stream) const override;
};

class FunctionDefineNode : public Expression {
    std::string name_;
    std::vector<std::string> arguments_names_;
    std::shared_ptr<Expression> expression_;

 public:
    FunctionDefineNode(std::string name, std::vector<std::string> arguments_names,
                       std::unique_ptr<Expression> expression, TokenRange range)
            : Expression(range), name_(std::move(name)), arguments_names_(std::move(arguments_names)),
              expression_(std::move(expression)) {}

    [[nodiscard]] const std::string &name() const { return name_; }
    [[nodiscard]] const std::vector<std::string> &arguments_names() const { return arguments_names_; }
    [[nodiscard]] const std::shared_ptr<Expression> &expression() const { return expression_; }

    BigDecimal eval(Context &context) override;
    void print(std::ostream &stream) const override;
};

#endif  // CALCULATOR_SRC_NODE_H
