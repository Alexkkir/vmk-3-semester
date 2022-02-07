// 4_1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define DEBUG 0

#if DEBUG
#include <stdio.h>
#endif


int satsum(int x1, int x2) {
    enum {
        _0,
        _1,
        _2,
        _4 = _2 + _2,
        _8 = _4 + _4,
        _32 = _8 * _4,
        _31 = _32 - _1,
        _INT_MIN = _1 << _31,
        _INT_MAX = ~_0 - _INT_MIN
    };

    int ans = x1 + x2;
    if (x1 > 0 && x2 > 0 && ans < 0) {
        return _INT_MAX;
    }
    else if (x1 < 0 && x2 < 0 && ans > 0) {
        return _INT_MIN;
    }
    else {
        return ans;
    }
    return 0;
}

#if DEBUG
int main()
{
    printf();
}
#endif

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
