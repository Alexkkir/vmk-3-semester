#include <limits.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>

typedef long long ll;

enum {
    BUF_LEN = 20,
    VAL2 = 17,
    VAL3 = 29,
};

int
main(int argc, char **argv)
{
    ll max[2] = {LLONG_MIN, LLONG_MIN};

    for (int nf = 1; nf < argc; nf++) {
        int fd = open(argv[nf], O_RDONLY);
        unsigned char buf[BUF_LEN];
        while(read(fd, buf, BUF_LEN) > 0) {
            unsigned char buf2[sizeof(int)];
            for (int i1 = 0, i2 = BUF_LEN - 1; i1 < sizeof(int); i1++, i2--) {
                buf2[i1] = buf[i2];
            }
            long long val = (long long) (int) *((int*) buf2);
            if (val > max[0]) {
                max[1] = max[0];
                max[0] = val;
            } else if (max[0] > val && val > max[1]) {
                max[1] = val;
            }
        }
        close(fd);
    }
    ll ans = max[1];
    if (ans == LLONG_MIN) return 0;
    if (ans < 0) {
        printf("-");
        ans *= -1;
    }
    ll n1, n2, n3;
    n3 = ans % VAL3;
    ans /= VAL3;
    n2 = ans % VAL2;
    ans /= VAL2;
    n1 = ans;
    printf("%lldg%02llds%02lldk\n", n1, n2, n3);
    return 0;
}