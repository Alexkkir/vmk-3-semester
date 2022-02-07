#include <stdio.h>

typedef int STYPE;
typedef unsigned int UTYPE;


STYPE bit_reverse(STYPE value) {
    UTYPE ans = 0;
    UTYPE help = value, help2 = ~((UTYPE) value);
//    for (int i = 0; (help) > 0 || (help2)> 0; i++) {
    for (int i = 0; (((UTYPE) value) >> i) > 0 || ((~((UTYPE) value)) >> i) > 0; i++) {
        ans <<= 1;
        ans |= ((UTYPE) value >> i) & 1;
        help >>= 1;
        help2 >>= 1;
        printf("%d\n", i);
    }

    return (STYPE) ans;
}

int main() {
    printf("%d", bit_reverse(-1));
    return 0;
}