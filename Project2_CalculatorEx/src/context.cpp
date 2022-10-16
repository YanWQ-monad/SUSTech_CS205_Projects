#include <algorithm>
#include <limits>

#include "context.h"
#include "constant.h"
#include "error.h"
#include "eval.h"

using std::endl;
using std::holds_alternative;
using std::max;
using std::ostream;
using std::string;
using std::to_string;
using std::vector;

// Reference: https://en.cppreference.com/w/cpp/utility/variant/visit
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

BigDecimal Function::invoke(const vector<Expression*> &arguments, Context &context) const {
    // guaranteed by caller
    assert(arguments.size() == arguments_names_.size());

    // clone a context, so inner function won't change outer context
    Context new_context = context;

    // we first evaluate the expressions into BigDecimal(s)
    // and put it to the context with corresponding name
    auto lhs = arguments_names_.begin();
    auto rhs = arguments.begin();
    for (; lhs != arguments_names_.end(); ++lhs, (void)++rhs)
        new_context.insert(*lhs, Entry::variable((*rhs)->eval(context)));

    // eval it
    return body_->eval(new_context);
}

BigDecimal LazyVariable::get_value(Context &context) const {
    // If no value yet, then calculate it
    if (!value_.has_value())
        value_ = evaluator_(context);

    return value_.value();
}

BigDecimal Entry::get_variable(Context &context, TokenRange caller) const {
    if (holds_alternative<Variable>(content_))
        return std::get<Variable>(content_).value();
    else if (holds_alternative<LazyVariable>(content_))
        return std::get<LazyVariable>(content_).get_value(context);

    throw ranged_error(caller, string("\"") + name_ + "\" is not a variable");
}

BigDecimal Entry::invoke_function(const vector<Expression*> &arguments, Context &context, TokenRange caller) const {
    return std::visit([&, this](auto &entry) {
        using T = std::decay_t<decltype(entry)>;

        // only `Function` or `BuiltinFunction` can be invoked
        if constexpr (std::is_same_v<T, Function> || std::is_same_v<T, BuiltinFunction>) {
            // check whether arguments number is correct
            if (entry.arguments_number() != arguments.size()) {
                string message = string("expected ") + to_string(entry.arguments_number()) + " arguments, "
                        + "but got " + to_string(arguments.size());
                throw ranged_error(caller, std::move(message));
            }

            // check overflow warning
            if (context.depth() >= kWarningDepth)
                throw stackoverflow_warning(caller, "stackoverflow warning");

            return entry.invoke(arguments, context);
        } else {
            throw ranged_error(caller, string("\"") + name_ + "\" is not a function");
            return BIG_DECIMAL_ZERO;  // dummy code, to make compiler correctly recognize the return type
        }
    }, content_);
}

const Entry& Context::get(const string &key, TokenRange caller) const {
    auto iter = map_.find(key);
    if (iter == map_.end())
        throw ranged_error(caller, string("no such variable or function: ") + key);
    return iter->second;
}

void Context::disable_depth_check() {
    // to disable the check, we can simply set it to -INF
    // so the condition `depth_ >= kWarningDepth` will never meet
    depth_ = std::numeric_limits<decltype(depth_)>::min();
}

void Context::insert(string key, Entry value) {
    value.name_ = key;
    map_.insert_or_assign(std::move(key), std::move(value));
}

bool Context::remove(const std::string &key) {
    return map_.erase(key) > 0;
}

void Context::print(ostream &stream) const {
    for (auto &[key, entry] : map_) {
        std::visit(overloaded {
            [&stream, &key = key](const Variable &v) {
                stream << "(variable) " << key << " = " << v.value();
            },
            [&stream, &key = key](const BuiltinFunction &) {
                stream << "(function) " << key << " = <built-in function>";
            },
            [&stream, &key = key](const Function &f) {
                stream << "(function) " << key << " = ";
                f.body()->print(stream);
            },
            [&stream, &key = key](const LazyVariable &v) {
                stream << "(variable) " << key << " = ";
                if (auto value = v.try_value(); value.has_value())
                    stream << value.value();
                else
                    stream << "<not evaluated yet>";
            },
        }, entry.content_);

        stream << endl;
    }
}

void load_builtin_context(Context &context) {
    context.insert("sqrt", Entry::builtin_function([](auto args, Context &context) {
        return sqrt(args[0]->eval(context), context.scale());
    }, 1));

    context.insert("if", Entry::builtin_function([](auto args, Context &context) {
        return ((args[0]->eval(context) != BIG_DECIMAL_ZERO) ? args[1] : args[2]) -> eval(context);
    }, 3));

    context.insert("floor", Entry::builtin_function([](auto args, Context &context) {
        BigDecimal result = args[0]->eval(context);
        result.drop_decimal();
        return result;
    }, 1));

    context.insert("round", Entry::builtin_function([](auto args, Context &context) {
        BigDecimal result = args[0]->eval(context);
        result.round_by_scale(context.scale());
        return result;
    }, 1));

    context.insert("pow", Entry::builtin_function([](auto args, Context &context) {
        BigDecimal lhs = args[0]->eval(context);
        BigDecimal rhs = args[1]->eval(context);
        if (rhs.exponent() < 0)
            throw ranged_error(args[1]->range(), "pow[x, y] only accept integer y, or try powf[x, y] instead");

        return pow(lhs, rhs, context.scale());
    }, 2));

    context.insert("powf", Entry::builtin_function([](auto args, Context &context) {
        BigDecimal lhs = args[0]->eval(context);
        BigDecimal rhs = args[1]->eval(context);

        BigDecimal tmp = pow(lhs, rhs, context.scale());
        size_t scale = max(0LL, tmp.most_significant_exponent()) + context.scale() + kExtraScale;

        return exp(rhs * ln(lhs, scale), context.scale());
    }, 2));

    context.insert("sin", Entry::builtin_function([](auto args, Context &context) {
        return sin(args[0]->eval(context), context.scale());
    }, 1));

    context.insert("cos", Entry::builtin_function([](auto args, Context &context) {
        return cos(args[0]->eval(context), context.scale());
    }, 1));

    context.insert("arctan", Entry::builtin_function([](auto args, Context &context) {
        return arctan(args[0]->eval(context), context.scale());
    }, 1));

    context.insert("exp", Entry::builtin_function([](auto args, Context &context) {
        return exp(args[0]->eval(context), context.scale());
    }, 1));

    context.insert("ln", Entry::builtin_function([](auto args, Context &context) {
        return ln(args[0]->eval(context), context.scale());
    }, 1));

    context.insert("phi", Entry::builtin_function([](auto args, Context &context) {
        return phi(args[0]->eval(context), context.scale());
    }, 1));

    context.insert("unset", Entry::builtin_function([](auto args, Context &context) {
        if (auto *identifier = dynamic_cast<VariableNode*>(args[0]))
            return context.remove(identifier->name()) ? BIG_DECIMAL_ONE : BIG_DECIMAL_ZERO;
        else
            throw ranged_error(args[0]->range(), "expected an identifier");
    }, 1));

    context.insert("pi", Entry::lazy_variable([](Context &context) {
        return pi(context.scale());
    }));

    context.insert("e", Entry::lazy_variable([](Context &context) {
        return exp(BIG_DECIMAL_ONE, context.scale());
    }));
}
