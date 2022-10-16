#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "error.h"
#include "token.h"

using std::back_inserter;
using std::find_if_not;
using std::string;
using std::string_view;
using std::vector;

inline bool is_punctuator(char ch) {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '%' || ch == '(' || ch == ')'
        || ch == '[' || ch == ']' || ch == ',' || ch == '<' || ch == '>' || ch == '=' || ch == ';';
}

inline bool is_space(char ch) {
    return ch == ' ' || ch == '\t';
}

inline bool is_identifier(char ch) {
    return isalnum(ch) || ch == '_';
}

inline bool is_identifier_first(char ch) {
    return isalpha(ch) || ch == '_';
}

enum TokenType {
    WhitespaceType,
    PunctuatorType,
    ValueType,
};

Token parse_string(string_view part, TokenType type, TokenRange range) {
    if (type == PunctuatorType) {
        return {part[0], range};
    } else if (type == ValueType) {
        if (is_identifier_first(part[0]) && find_if_not(part.begin(), part.end(), is_identifier) == part.end()) {
            return {string(part), range};
        } else {
            try {
                return {BigDecimal(part), range};
            } catch (number_parse_error &e) {
                throw ranged_error(range, "expected a valid number or identifier", e.what());
            }
        }
    } else {
        assert(false && "unreachable");
        __builtin_unreachable();
    }
}

vector<Token> tokenize(string_view input, size_t frame_id) {
    vector<Token> result;

    TokenType type = WhitespaceType;
    bool last_is_e = false;  // indicate whether the previous character is 'e'

    // `i` is current position, `j` is the beginning of this chunk
    for (size_t i = 0, j = 0; i <= input.length(); i++) {
        char ch = i < input.length() ? input[i] : ' ';  // space as guard in the end

        // check whether the character is valid
        if (!(isalnum(ch) || ch == '_' || ch == '.' || ch == ' ' || is_punctuator(ch))) {
            string message = string("'") + ch + "' came as a complete surprise to me";
            throw ranged_error(TokenRange::single(i, frame_id), std::move(message));
        }

        TokenType next_type = is_space(ch) ? WhitespaceType : (is_punctuator(ch) ? PunctuatorType : ValueType);
        if (last_is_e && ch == '-')
            next_type = ValueType;  // deal with number like '1e-15'

        // if the type changes, or is Punctuator (Punctuator should not be grouped to chunk)
        if (type != next_type || type == PunctuatorType) {
            if (type != WhitespaceType)
                result.push_back(parse_string(input.substr(j, i - j), type, TokenRange{j, i, frame_id}));
            j = i;
            type = next_type;
        }

        last_is_e = ch == 'e';
    }

    return result;
}
