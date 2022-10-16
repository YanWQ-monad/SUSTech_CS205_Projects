#include <algorithm>
#include <string>
#include <vector>

#include "constant.h"
#include "context.h"
#include "error.h"
#include "node.h"

using std::back_inserter;
using std::ostream;
using std::string;
using std::transform;
using std::vector;

BigDecimal VariableNode::eval(Context &context) {
    return context.get(name_, range_).get_variable(context, range_);
}

BigDecimal BinOpNode::eval_wrapper(Context &context) {
    switch (type_) {
        case BinOp_ADD:
            return lhs_->eval(context) + rhs_->eval(context);
        case BinOp_SUB:
            return lhs_->eval(context) - rhs_->eval(context);
        case BinOp_MUL:
            return lhs_->eval(context) * rhs_->eval(context);
        case BinOp_DIV:
            return lhs_->eval(context).div_with_scale(rhs_->eval(context), context.scale());
        case BinOp_MOD:
            return lhs_->eval(context) % rhs_->eval(context);
        case BinOp_LE:
            return lhs_->eval(context) < rhs_->eval(context) ? BIG_DECIMAL_ONE : BIG_DECIMAL_ZERO;
        case BinOp_GE:
            return lhs_->eval(context) > rhs_->eval(context) ? BIG_DECIMAL_ONE : BIG_DECIMAL_ZERO;
        default:
            throw ranged_error(range_, string("unexpected operator: ") + static_cast<char>(type_));
    }
}

BigDecimal BinOpNode::eval(Context &context) {
    BigDecimal result = eval_wrapper(context);
    if (!context.disabled_divergent_check() && result.mantissa().digits().size() > kDivergentLimit)
        throw divergent_warning(range_, "divergent warning");
    return result;
}

BigDecimal SequenceNode::eval(Context &context) {
    lhs_->eval(context);
    return rhs_->eval(context);
}

BigDecimal FunctionNode::eval(Context &context) {
    vector<Expression*> arguments;
    transform(args_.begin(), args_.end(), back_inserter(arguments),
              [](auto &expression) { return expression.get(); });
    return context.get(name_, range_).invoke_function(arguments, context, range_);
}

BigDecimal AssignmentNode::eval(Context &context) {
    BigDecimal result = expression_->eval(context);
    context.insert(name_, Entry::variable(result));
    return result;
}

BigDecimal FunctionDefineNode::eval(Context &context) {
    context.insert(name_, Entry::function(arguments_names_, expression_));
    return BIG_DECIMAL_ZERO;
}

void NumericNode::print(ostream &stream) const {
    stream << number_;
}

void VariableNode::print(ostream &stream) const {
    stream << name_;
}

void BinOpNode::print(ostream &stream) const {
    stream << "(";
    lhs_->print(stream);
    stream << " " << static_cast<char>(type_) << " ";
    rhs_->print(stream);
    stream << ")";
}

void SequenceNode::print(ostream &stream) const {
    stream << "(";
    lhs_->print(stream);
    stream << "; ";
    rhs_->print(stream);
    stream << ")";
}

void FunctionNode::print(ostream &stream) const {
    stream << name_ << "[";

    bool is_first = true;
    for (const auto &arg : args_) {
        if (!is_first)
            stream << ", ";
        is_first = false;

        arg->print(stream);
    }
    stream << "]";
}

void AssignmentNode::print(ostream &stream) const {
    stream << name_ << " = ";
    expression_->print(stream);
}

void FunctionDefineNode::print(ostream &stream) const {
    stream << name_ << "";

    bool is_first = true;
    for (auto &x : arguments_names_) {
        stream << (is_first ? "[" : ", ") << x;
        is_first = false;
    }

    stream << "] = ";
    expression_->print(stream);
}
