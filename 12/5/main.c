#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
//#include <sys/wait.h>
#include <signal.h>
#include <math.h>

enum { N_PRINTS = 3, BRUTE_FORCE_START_VAL = 2, DELTA = 2, };

volatile unsigned long long last_prime = 0;
volatile int counter = 0;

void handler(int s) {
    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    if (s == SIGINT) {
        if (counter == N_PRINTS)
            _exit(0);
        printf("%lld\n", last_prime);
        fflush(stdout);
        counter++;
    } else {
        _exit(0);
    }
}

int is_prime(unsigned long long x) {
    if (x == 0 || x == 1 || (x & 1) == 0)
        return 0;
    int lim = (int) sqrt(x) + DELTA;
    for (unsigned long long div = BRUTE_FORCE_START_VAL; div <= lim; div++) {
        if (x % div == 0)
            return 0;
    }
    return 1;
}

int main() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    unsigned long long low, high;
    scanf("%lld%lld", &low, &high);

    printf("%d\n", getpid());
    fflush(stdout);

    signal(SIGINT, handler);
    signal(SIGTERM, handler);

    unsigned long long x;
    for (x = low; x < high; x++) {
        if (is_prime(x)) {
            last_prime = x;
        }
    }

    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    if (x == high) {
        printf("-1\n");
        fflush(stdout);
    }

    return 0;
}
