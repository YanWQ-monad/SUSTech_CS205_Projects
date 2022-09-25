#include "mul.cpp"
using namespace std;

extern "C" {

struct c_biginteger {
    BigInteger *inner;
};

struct c_bigdecimal {
    BigDecimal *inner;
};

const int MAX_LENGTH = 5000000;
char buffer[MAX_LENGTH * 2];

c_biginteger *create_biginteger(const char *number) {
    auto *res = new c_biginteger;
    try {
        res->inner = new BigInteger(number);
    } catch (number_parse_error &) {
        delete res;
        return nullptr;
    }
    return res;
}

c_biginteger *integer_multiplication(c_biginteger *lhs, c_biginteger *rhs) {
    BigInteger result = *lhs->inner * *rhs->inner;
    auto *ptr = new c_biginteger;
    ptr->inner = new BigInteger(std::move(result));
    return ptr;
}

char *integer_string(c_biginteger *integer) {
    string s = integer->inner->get_number_string();
    strncpy(buffer, s.c_str(), sizeof(buffer) - 1);
    return buffer;
}

void delete_integer(c_biginteger *ptr) {
    delete ptr->inner;
    delete ptr;
}

c_bigdecimal *create_bigdecimal(const char *number) {
    auto *res = new c_bigdecimal;
    try {
        res->inner = new BigDecimal(number);
    } catch (number_parse_error &) {
        delete res;
        return nullptr;
    }
    return res;
}

c_bigdecimal *decimal_multiplication(c_bigdecimal *lhs, c_bigdecimal *rhs) {
    BigDecimal result = *lhs->inner * *rhs->inner;
    auto *ptr = new c_bigdecimal;
    ptr->inner = new BigDecimal(std::move(result));
    return ptr;
}

char *decimal_string(c_bigdecimal *ptr) {
    ostringstream ss;
    ss << *ptr->inner;
    strncpy(buffer, ss.str().c_str(), sizeof(buffer) - 1);
    return buffer;
}

void delete_decimal(c_bigdecimal *ptr) {
    delete ptr->inner;
    delete ptr;
}

}
