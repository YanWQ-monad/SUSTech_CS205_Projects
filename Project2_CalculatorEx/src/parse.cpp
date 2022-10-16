#include <memory>
#include <stack>
#include <optional>
#include <tuple>
#include <utility>
#include <vector>

#include "error.h"
#include "node.h"
#include "parse.h"
#include "token.h"

using std::find;
using std::make_pair;
using std::make_tuple;
using std::make_unique;
using std::optional;
using std::pair;
using std::reverse;
using std::stack;
using std::string;
using std::string_view;
using std::tuple;
using std::variant;
using std::vector;

int get_precedence(Punctuator ch) {
    switch (ch) {
        case '(':
        case '[':
        case ',':
            return 1;
        case ';':
            return 2;
        case '=':
            return 3;
        case '<':
        case '>':
            return 4;
        case '+':
        case '-':
            return 5;
        case '*':
        case '/':
        case '%':
            return 6;
        default:
            throw runtime_error(string("unexpected operator: ") + ch);
    }
}

// (try to) extract an identifier from the given Expression
// used in assignment classification (variable assignment or function declaration)
optional<string> extract_identifier(const ExpressionStm &expression) {
    // just a dynamic_cast can do it
    if (auto *node = dynamic_cast<VariableNode*>(expression.get()))
        return string(node->name());
    return {};
}

// (try to) extract a function signature (like "f[x, y]") from the given Expression
// used in assignment classification (variable assignment or function declaration)
// return type: optional of (name, arguments)
optional<tuple<string, vector<string>>> extract_function_signature(ExpressionStm &expression) {
    // first, it must be a `FunctionNode`
    if (auto *node = dynamic_cast<FunctionNode*>(expression.get())) {
        vector<string> arguments_names;
        auto &args = node->args();
        for (const auto &arg : args) {
            // then, each argument should be a valid identifier (not an expression)
            if (auto *arg_node = dynamic_cast<VariableNode*>(arg.get()))
                arguments_names.emplace_back(arg_node->name());
            else
                return {};
        }
        return make_tuple(node->name(), std::move(arguments_names));
    }

    return {};
}

// the state machine as introduced in the report
enum EntryType { TValue, TPunctuator };

class ExpressionStackHelper {
    stack<ExpressionStm> value_stack_;
    stack<pair<Punctuator, TokenRange>> operator_stack_;

    EntryType last_entry_type_ = TPunctuator;  // start from Punctuator

    void update_top_range(TokenRange range) {
        value_stack_.top()->range() = range;
    }

 public:
    // pop a value from stack with empty check
    ExpressionStm pop_value(TokenRange caller) {
        if (value_stack_.empty())
            throw ranged_error(caller, "no enough value for the operator");
        ExpressionStm ptr = std::move(value_stack_.top());
        value_stack_.pop();
        return ptr;
    }

    // pop an operator from stack, caller should ensure the stack is not empty
    pair<Punctuator, TokenRange> pop_operator() {
        auto op = std::move(operator_stack_.top());
        operator_stack_.pop();
        return op;
    }

    void merge_top_operation() {
        auto [op, range] = pop_operator();

        // '(' '[' ',' should only be handled by ')' or ']', otherwise it's invalid
        if (op == '(' || op == '[')
            throw ranged_error(range, "unclosed bracket");
        if (op == ',')
            throw ranged_error(range, "lonely comma (should only be used in function call)");

        // pop two value from stack, since it's a binary operator
        ExpressionStm rhs = pop_value(range);
        ExpressionStm lhs = pop_value(range);

        TokenRange new_range = lhs->range() + rhs->range();
        if (op == '=') {
            if (auto identifier = extract_identifier(lhs); identifier.has_value()) {
                // variable assignment
                push_value<AssignmentNode>(identifier.value(), std::move(rhs), new_range);
            } else if (auto function = extract_function_signature(lhs); function.has_value()) {
                // function declaration
                auto [name, arguments] = std::move(function.value());
                push_value<FunctionDefineNode>(name, arguments, std::move(rhs), new_range);
            } else {
                // neither of them, invalid assignment
                throw ranged_error(lhs->range(), "should be a valid function declaration or variable name");
            }
        } else if (op == ';') {
            // sequence expressions
            push_value<SequenceNode>(std::move(lhs), std::move(rhs), new_range);
        } else {
            // normal operator like "+-*/%"
            auto op_type = static_cast<BinOpNode::BinaryOperationType>(op);
            push_value<BinOpNode>(std::move(lhs), std::move(rhs), op_type, new_range);
        }
    }

    // check whether it should merge the top operator when the next operator is `op`
    [[nodiscard]] bool can_merge(Punctuator op) const {
        if (operator_stack_.empty())
            return false;

        int lhs = get_precedence(op), rhs = get_precedence(operator_stack_.top().first);
        if (lhs != rhs)
            return lhs < rhs;

        // assignment operation is Right-to-left Associativity,
        // so when '=' meets '=', we should not merge the former one.
        return op != '=';
    }

    // fold ')'
    void fold_parentheses(TokenRange end_range) {
        while (!operator_stack_.empty() && operator_stack_.top().first != '(')
            merge_top_operation();
        if (operator_stack_.empty())
            throw ranged_error(end_range, "no matching '(' was found");
        update_top_range(pop_operator().second + end_range);
    }

    // fold ']' to '[', return the arguments list
    vector<ExpressionStm> fold_square_brackets(TokenRange end_range) {
        vector<ExpressionStm> result;

        while (!operator_stack_.empty() && operator_stack_.top().first != '[') {
            if (operator_stack_.top().first == ',') {
                result.push_back(pop_value(end_range));
                pop_operator();
            } else {
                merge_top_operation();
            }
        }
        result.push_back(pop_value(end_range));

        if (operator_stack_.empty())
            throw ranged_error(end_range, "no matching '[' was found");
        pop_operator();
        reverse(result.begin(), result.end());
        return result;
    }

    // finish parsing and return the final expression
    ExpressionStm finalize() {
        if (last_entry_type_ != TValue)  // should ends with a value
            throw ranged_error(operator_stack_.top().second, "unexpected punctuator in the end");
        while (!operator_stack_.empty())
            merge_top_operation();
        assert(value_stack_.size() == 1);
        return std::move(value_stack_.top());
    }

    template<typename T, class... Args>
    void push_value(Args ...args) {
        value_stack_.push(make_unique<T>(std::forward<Args>(args)...));
    }

    void push_operator(Punctuator op, TokenRange range) {
        operator_stack_.push(make_pair(op, range));
    }

    // state checking and transition, `from` -> `to`
    void check_state_and_jump(EntryType from, EntryType to, TokenRange range) {
        if (last_entry_type_ != from) {
            const char *type_name = last_entry_type_ == TValue ? "value" : "punctuator";
            throw ranged_error(range, string("expected to be following by ") + type_name);
        }
        last_entry_type_ = to;
    }
};

class ExpressionVisitor {
    ExpressionStackHelper stack_;

 public:
    void operator()(BigDecimal decimal, TokenRange range) {
        stack_.check_state_and_jump(TPunctuator, TValue, range);
        stack_.push_value<NumericNode>(std::move(decimal), range);
    }

    void operator()(Identifier identifier, TokenRange range) {
        stack_.check_state_and_jump(TPunctuator, TValue, range);
        stack_.push_value<VariableNode>(std::move(identifier), range);
    }

    void operator()(Punctuator operation, TokenRange range) {
        if (operation == '(' || operation == '[' || operation == ',') {
            if (operation == '(')
                stack_.check_state_and_jump(TPunctuator, TPunctuator, range);
            else
                stack_.check_state_and_jump(TValue, TPunctuator, range);

            stack_.push_operator(operation, range);
        } else if (operation == ')') {
            stack_.check_state_and_jump(TValue, TValue, range);

            stack_.fold_parentheses(range);
        } else if (operation == ']') {
            stack_.check_state_and_jump(TValue, TValue, range);

            // first fold ']' to get arguments list
            vector<ExpressionStm> args = stack_.fold_square_brackets(range);

            // then try to extract the function name
            ExpressionStm name = stack_.pop_value(range);
            if (auto *node = dynamic_cast<VariableNode *>(name.get())) {
                string_view identifier = node->name();
                stack_.push_value<FunctionNode>(string(identifier), std::move(args), node->range() + range);
            } else {
                throw ranged_error(name->range(), "expected an identifier");
            }
        } else {
            // normal operator like "+-*/%"
            stack_.check_state_and_jump(TValue, TPunctuator, range);
            while (stack_.can_merge(operation))
                stack_.merge_top_operation();
            stack_.push_operator(operation, range);
        }
    }

    ExpressionStm finalize() {
        return stack_.finalize();
    }
};

ExpressionStm parse(string_view input, size_t frame_id) {
    // 1. tokenize
    auto tokens = tokenize(input, frame_id);
    if (tokens.empty())
        throw ranged_error(TokenRange{0, input.length(), frame_id}, "empty expression");

    // 2. parse
    ExpressionVisitor visitor;
    for (Token &item : tokens)
        std::visit(visitor, std::move(item.content()), variant<TokenRange>(item.range()));
    return visitor.finalize();
}
