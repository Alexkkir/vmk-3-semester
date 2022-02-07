#include <stdio.h>

enum {
    NEUTRAL = 0,
};

int get_num(char *str) {
    // parsing ...
    int negative, substract, thousnads;
    substract = 1;
}

int main(int argc, char **argv) {
    int ans = NEUTRAL;
    for (int i = 1; i < argc; i++) {
        ans += get_num(argv[i]);
    }
    printf("%d\n", ans);
    return 0;
}
