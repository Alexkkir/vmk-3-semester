#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    int argc = 5;
    char* argv[] = {"as", "100.0", "-10.0", "-5.5", "1.0"};
    char *ptr;
    double ans = strtod(argv[1], &ptr);
    for (int i = 2; i < argc; i++) {
        double x = strtod(argv[i], &ptr);
        double delta = ans * (x / 100.0);
        ans += delta;
        char *a = (char*) malloc(10 * sizeof(char));
        sscanf("abcdefg", "%s", a);
        ans *= 10000.0;
        ans = roundl(ans);
        ans /= 10000.0;
    }
    printf("%.4lf\n", ans);
    return 0;
}
