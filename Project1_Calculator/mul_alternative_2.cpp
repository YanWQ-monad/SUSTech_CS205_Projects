#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
    if (argc != 3) {
        cout << "no enough arguments" << endl;
        return 1;
    }

    try {
        double lhs = stod(argv[1]), rhs = stod(argv[2]);
        double result = lhs * rhs;

        cout << lhs << " * " << rhs << " = " << result << endl;
    }
    catch (logic_error &e) {
        cout << "Failed: " << e.what() << endl;
        return 1;
    }

    return 0;
}
