#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "no enough arguments" << endl;
        return 1;
    }

    try {
        int64_t lhs = stoll(argv[1]), rhs = stoll(argv[2]);
        int64_t result = lhs * rhs;

        if (lhs == 0 || result / lhs == rhs)
            cout << lhs << " * " << rhs << " = " << result << endl;
        else
            throw out_of_range("multiplication overflow");
    }
    catch (logic_error &e) {
        cout << "Failed: " << e.what() << endl;
        return 1;
    }

    return 0;
}
