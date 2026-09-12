
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

int subtractNumbers(string expression) {
    int difference = 0;
    int num = 0;
    bool firstNumber = true;

    for (char c : expression) {
        if (c == '-') {
            if (firstNumber) {
                difference = num;
                firstNumber = false;
            } else {
                difference -= num;
            }
            num = 0;
        } else {
            num = num * 10 + (c - '0');
        }
    }

    if (firstNumber) {
        difference = num;
    } else {
        difference -= num;
    }

    return difference;
}

int main() {
    string expression;
    cin >> expression;

    if (expression.find('-') != string::npos) {
        cout << subtractNumbers(expression) << endl;
    } else {
        cout << addNumbers(expression) << endl;
    }

    return 0;
}
