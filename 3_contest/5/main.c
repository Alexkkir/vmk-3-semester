#include <limits.h>
#include <stdio.h>
#include <string.h>

/*
 * To test type:     vival 5.exe -t test.txt
 */

enum my_enum {
    radix = 10,
    SIZE_BUF = PATH_MAX + 10
};
typedef unsigned long long ull;

void
print_empty() {
    for (int i = 0; i < RADIX; i++) {
        printf("%d %d\n", i, 0);
    }
}

void
print_arr(const ull count[]) {
    for (int i = 0; i < RADIX; i++) {
        printf("%d %lld\n", i, count[i]);
    }
}

int
main() {
    char name[SIZE_BUF];

    fgets(name, SIZE_BUF, stdin);
    unsigned len = strlen(name);
    if (len == 0) {
        print_empty();
        return 0;
    } else {
        while (name[len - 1] == '\n' || name[len - 1] == '\r') {
            name[len - 1] = '\0';
            len--;
        }
    }

    FILE *f = fopen(name, "r");
    if (!f) {
        print_empty();
    } else {
        ull count[RADIX];
        for (int i = 0; i < RADIX; i++) {
            count[i] = 0;
        }

        int c;
        while ((c = getc(f)) != EOF) {
            if ('0' <= c && c <= '9') {
                count[c - '0']++;
            }
        }

        print_arr(count);
    }
    return 0;
}
