#include <unistd.h>  // isatty(fd)
#include <iostream>
#include <utility>

#include "constant.h"
#include "context.h"
#include "error.h"
#include "parse.h"

using std::cin;
using std::cerr;
using std::cout;
using std::endl;
using std::holds_alternative;
using std::string;
using std::vector;

struct options {
    size_t scale = 20;
    bool disable_depth_check = false;
    bool disable_divergent_check = false;
};

void print_help(const char *executable) {
    cout << "USAGE: " << executable << " [options...]" << endl;
    cout << R"(A Calculator similar to bc

OPTIONS:
      --no_depth_check      Disable recursion depth check
      --no_divergent_check  Disable divergent check
  -s, --scale <N>           Set scale to N (default 20)

BUILTIN FUNCTIONS AND VARIABLES:
  floor[x]                  Return the floor of x as an integral
  round[x]                  Return the rounded value of x to the set scale
  if[cond, x1, x2]          If cond is not zero, x1 is returned, otherwise is x2
  unset[name]               Remove a function or variable called name
  arctan[x]                 Return the arc tangent of x
  cos[x]                    Return the cosine of x
  exp[x]                    Return e raised to the power of x
  ln[x]                     Return the natural logarithm of x
  phi[x]                    Return the value of CDF of the standard normal distribution at x
  pow[x, y]                 Return x raised to the power of y (only accept integer y)
  powf[x, y]                Return x raised to the power of y (same as pow[x, y] but accept decimal y)
  sin[x]                    Return the sine of x
  sqrt[x]                   Return the square root of x
  e                         Euler's number
  pi                        PI
)";
}

options parse_options(int argc, char *argv[]) {
    options option;

    if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
        print_help(argv[0]);
        exit(0);
    }

    for (int i = 1; i < argc; i++) {
        if ((!strcmp("-s", argv[i]) || !strcmp("--scale", argv[i])) && i + 1 < argc) {
            try {
                option.scale = std::stoll(argv[i + 1]);
                i++;
                continue;
            }
            catch (std::logic_error &) {
                cerr << "Failed to parse " << argv[i + 1] << " to integer" << endl;
                exit(1);
            }
        }

        if (!strcmp("--no_depth_check", argv[i])) {
            option.disable_depth_check = true;
            continue;
        }

        if (!strcmp("--no_divergent_check", argv[i])) {
            option.disable_divergent_check = true;
            continue;
        }

        cerr << "Unrecognized option: " << argv[i] << endl;
        cerr << "Try \"" << argv[0] << " --help\" for more information" << endl;
        exit(1);
    }

    return option;
}

void print_ranged_message(TokenRange range, const vector<string> &inputs) {
    if (range.frame_id_ != inputs.size()) {
        cerr << "Input #" << range.frame_id_ << ":" << endl;
        cerr << "  " << inputs[range.frame_id_] << endl;
    }

    std::fill_n(std::ostream_iterator<char>(cerr), range.begin_ + 2, ' ');
    std::fill_n(std::ostream_iterator<char>(cerr), range.end_ - range.begin_, '~');
    cerr << endl;
}

int main(int argc, char *argv[]) {
    // since we do not use C-like IO function, we can safely disable the sync
    std::ios_base::sync_with_stdio(false);

    // initialize the context (with options)
    options option = parse_options(argc, argv);

    Context context;
    context.scale() = option.scale;
    load_builtin_context(context);

    if (option.disable_depth_check)
        context.disable_depth_check();
    if (option.disable_divergent_check)
        context.disabled_divergent_check() = true;

    bool interactive = isatty(STDIN_FILENO);

    // main loop
    vector<string> inputs;
    size_t frame_id = 0;
    while (true) {
        if (interactive)
            cout << "> ";
        string input;
        getline(cin, input);

        if (cin.eof())
            break;
        if (input.empty())
            continue;

        if (input == "env") {
            context.print(cout);
            continue;
        }

        try {
            ExpressionStm parse_result = parse(input, frame_id);
            BigDecimal eval_result = parse_result->eval(context);
            if (!dynamic_cast<FunctionDefineNode*>(parse_result.get()))
                cout << eval_result << endl;
        } catch (stackoverflow_warning &e) {
            print_ranged_message(e.range(), inputs);
            cerr << "Error: Your recursion depth is exceeded " << kWarningDepth << endl;
            cerr << "If you are sure what to do, you can disable the check with \"--no_depth_check\"" << endl;
        } catch (divergent_warning &e) {
            print_ranged_message(e.range(), inputs);
            cerr << "Error: One decimal is exceeded " << kDivergentLimit << " digits, maybe is divergent" << endl;
            cerr << "If you are sure what to do, you can disable the check with \"--no_divergent_check\"" << endl;
        } catch (ranged_error &e) {
            print_ranged_message(e.range(), inputs);
            cerr << "Error: " << e.what() << endl;
        } catch (application_error &e) {
            cerr << "Error: " << e.what() << endl;
        }

        inputs.push_back(input);
        frame_id++;
    }

    return 0;
}
