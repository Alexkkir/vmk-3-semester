#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <unistd.h>

typedef long long ll;

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDWR);
    long filesize = lseek(fd, 0, SEEK_END);
    if (filesize == 0) {
        return 0;
    }
    lseek(fd, 0, SEEK_SET);
    ll min = LLONG_MAX;
    int pos_min = 0;
    for (long i = 0; i < filesize; i += sizeof(ll)) {
        ll val;
        read(fd, &val, sizeof(val));
        if (val < min) {
            min = val;
            pos_min = i;
        }
    }
    if (min == LLONG_MIN) {
        return 0;
    }
    lseek(fd, pos_min, SEEK_SET);
    ll t = ~min + 1;
    write(fd, &t, sizeof(t));
    return 0;
}