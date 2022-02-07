#include <stdio.h>

typedef long long ll;

enum {
    SHIFT = 8,
    LEN = 9,
};

int
main(int argc, char **argv)
{
    const char sample[] = "rwxrwxrwx";
    char ans[LEN + 1] = {0};
    for (int i = 1; i < argc; i++) {
        int num = 0;
        sscanf(argv[i], "%o", &num);
        int bit = 1 << SHIFT;
        for (int j = 0; j < LEN; j++, bit >>= 1) {
            if ((num & bit) == 0) {
                ans[j] = '-';
            } else {
                ans[j] = sample[j];
            }
        }
        printf("%s\n", ans);
    }
    return 0;
}