#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <limits.h>

enum { MAX_LEN = 10000, RADIX = 3 };

typedef unsigned long long ull;
typedef long long ll;

uint64_t get_d(char c) {
    if (c == '0') {
        return 0;
    } else if (c == '1') {
        return 1;
    } else if (c == 'a') {
        return -1;
    }
    return 0;
}

void operate(char *str) {
    ull ans = 0, ans_prev;
    int len = strlen(str), overflow = 0;
    for (int i = 0; i < len && !overflow; i++) {
        ans_prev = ans;
        ans *= RADIX;
        ans += get_d(str[i]);
        if ( (((ll) ans > 0) && ((ll) ans_prev < 0)) || (((ll) ans < 0) && ((ll) ans_prev > 0)) ) {
            overflow = 1;
        }
    }
    if (overflow) {
        printf("18446744073709551616");
    } else {
        printf("%lld\n", (ll) ans);
    }
}

int main() {
    char str[MAX_LEN];

    while (scanf("%s", str) > 0) {
        operate(str);
    }

    return 0;
}
