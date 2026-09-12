#include <iostream>
using namespace std;

int addNumbers(string expression) {
    int sum = 0;
    int num = 0;

    for (char c : expression) {
        if (c == '+') {
            sum += num;
            num = 0;
        } else {
            num = num * 10 + (c - '0');
        }
    }

    sum += num;

    return sum;
}

int main() {
    string expression;
    cin >> expression;

    cout << addNumbers(expression) << endl;

    return 0;
}
