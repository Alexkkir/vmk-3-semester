#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>

typedef unsigned long long ull;

enum {
    MS = 1000000,
};

int main(int argc, char **argv) {
    struct timeval time;
    sscanf(argv[1], "%ld", &time.tv_sec);
    sscanf(argv[2], "%ld", &time.tv_usec);

    double lambda;
    int k;
    unsigned seed;

    sscanf(argv[3], "%lf", &lambda);
    sscanf(argv[4], "%d", &k);
    sscanf(argv[5], "%u", &seed);

    srand(seed);
    for (int i = 0; i < k; i++) {
        double x = (double) rand() / RAND_MAX;
        double p = -log(x) / lambda;
        long a = (int) p;
        time.tv_usec += a;
        time.tv_sec += time.tv_usec / MS;
        time.tv_usec %= MS;
        printf("%ld %ld\n", time.tv_sec, time.tv_usec);
    }

    return 0;
}
