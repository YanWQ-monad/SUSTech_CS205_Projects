#ifndef CALCULATOR_SRC_CONTEXT_H
#define CALCULATOR_SRC_CONTEXT_H

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "node.h"
#include "number.h"

class Context;

class Variable {
    BigDecimal value_;

 public:
    explicit Variable(BigDecimal value) : value_(std::move(value)) {}

    [[nodiscard]] const BigDecimal& value() const { return value_; }
};

class Function {
    std::vector<std::string> arguments_names_;
    std::shared_ptr<Expression> body_;

 public:
    Function(std::vector<std::string> arguments_names, std::shared_ptr<Expression> body)
            : arguments_names_(std::move(arguments_names)), body_(std::move(body)) {}

    [[nodiscard]] const std::shared_ptr<Expression> &body() const { return body_; }
    [[nodiscard]] size_t arguments_number() const { return arguments_names_.size(); }

    BigDecimal invoke(const std::vector<Expression*> &arguments, Context &context) const;
};

class BuiltinFunction {
    std::function<BigDecimal(const std::vector<Expression*>, Context &)> body_;
    size_t arguments_number_;

 public:
    BuiltinFunction(std::function<BigDecimal(std::vector<Expression*>, Context &)> body, size_t arguments_number)
            : body_(std::move(body)), arguments_number_(arguments_number) {}

    [[nodiscard]] size_t arguments_number() const { return arguments_number_; }

    BigDecimal invoke(const std::vector<Expression*> &arguments, Context &context) const {
        return body_(arguments, context);
    }
};

class LazyVariable {
    std::function<BigDecimal(Context &)> evaluator_;
    mutable std::optional<BigDecimal> value_;

 public:
    explicit LazyVariable(std::function<BigDecimal(Context &)> evaluator)
            : evaluator_(std::move(evaluator)) {}

    [[nodiscard]] BigDecimal get_value(Context &context) const;
    [[nodiscard]] std::optional<BigDecimal> try_value() const { return value_; }
};

class Entry {
    using ContentType = std::variant<Variable, Function, BuiltinFunction, LazyVariable>;
    ContentType content_;
    std::string name_;  // set by `Context` when it's inserted to context

    explicit Entry(ContentType content) : content_(std::move(content)) {}

 public:
    template<class... Args>
    static Entry variable(Args ...args) {
        return Entry(Variable(std::forward<Args>(args)...));
    }

    template<class... Args>
    static Entry function(Args ...args) {
        return Entry(Function(std::forward<Args>(args)...));
    }

    template<class... Args>
    static Entry builtin_function(Args ...args) {
        return Entry(BuiltinFunction(std::forward<Args>(args)...));
    }

    template<class... Args>
    static Entry lazy_variable(Args ...args) {
        return Entry(LazyVariable(std::forward<Args>(args)...));
    }

    [[nodiscard]] const ContentType& content() const { return content_; }

    [[nodiscard]] BigDecimal get_variable(Context &context, TokenRange caller) const;
    BigDecimal invoke_function(const std::vector<Expression*> &arguments, Context &context, TokenRange caller) const;

    friend class Context;
};

class Context {
    std::unordered_map<std::string, Entry> map_{};
    size_t scale_ = 20;
    int64_t depth_ = 0;
    bool disabled_divergent_check_ = false;

 public:
    Context() = default;
    Context(const Context &context) : map_(context.map_), scale_(context.scale_), depth_(context.depth_ + 1) {}

    [[nodiscard]] const Entry& get(const std::string &key, TokenRange caller) const;
    [[nodiscard]] int64_t depth() const { return depth_; }
    [[nodiscard]] size_t scale() const { return scale_; }
    [[nodiscard]] bool disabled_divergent_check() const { return disabled_divergent_check_; }
    size_t& scale() { return scale_; }
    bool& disabled_divergent_check() { return disabled_divergent_check_; }

    void disable_depth_check();
    void insert(std::string key, Entry value);
    bool remove(const std::string &key);  // return true if success, false if no such key
    void print(std::ostream &stream) const;
};

void load_builtin_context(Context &context);

#endif  // CALCULATOR_SRC_CONTEXT_H
