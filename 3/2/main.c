#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
    long long int sump = 0, sumn = 0;

    for (int i = 1; i < argc; i++) {
        int x = 0;
        x = atoi(argv[i]);
        if (x > 0) {
            sump += x;
        } else {
            sumn += x;
        }
    }
    printf("%lld\n%lld\n", sump, sumn);
    return 0;
}
