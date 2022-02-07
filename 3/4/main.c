#define READY 1

#if !READY
#include <stdio.h>
#endif


int satsum(int v1, int v2) {
    int _0 = 0;
    int _1 = ++_0;
    int _2 = ++_1;
    _0--; _1--;
    int _4 = _2 + _2,
        _8 = _4 + _4,
        _32 = _8 * _4,
        _31 = _32 - _1,
        _int_min = (int) ((unsigned) _1 << (unsigned)_31),
        _int_max = (int) (((unsigned) ~_0) - (unsigned) _int_min);


    int ans;
    if (v1 > 0 && v2 > 0) {
        ans = (int) ((unsigned) v1 + (unsigned) v2);
        if (ans < 0) {
            return _int_max;
        } else {
            return ans;
        }
    }
    else if (v1 < 0 && v2 < 0) {
        unsigned h1 = v1, h2 = v2;
        h1 -= _int_min;
        h2 -= _int_min;
        ans = (int) (h1 + h2);
        if (ans > 0) {
            return _int_min;
        } else {
            return ans;
        }
    }
    else {
        return v1 + v2;
    }
    return 0;
}

#if !READY
int main()
{
    int a, b;
    scanf("%d%d", &a, &b);
    printf("%d\n", satsum(a, b));
}
#endif