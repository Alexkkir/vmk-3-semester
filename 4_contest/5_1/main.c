#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <unistd.h>

typedef unsigned long long ull;
typedef long long ll;

enum {
    NUM_SIZE = sizeof(ll),
};

int main(int argc, char* argv[]) {
//    char *name = "input.bin";

    int fd = open(argv[1], O_RDWR);
    ll filesize = lseek(fd, 0, SEEK_END);

    ll min = LLONG_MAX;
    long pos_min = 0;
    lseek(fd, 0, SEEK_SET);

    for (int i = 0; i < filesize; i += NUM_SIZE) {
        unsigned char buf[NUM_SIZE];
        ll a;
        ssize_t count;
        int tmp = 0;
        while ((count = read(fd, buf + tmp, NUM_SIZE - tmp)) > 0){
            tmp += count;
            if (tmp == 4){
                break;
            }
        }
        a = *((ll*)buf);
        if (min - a > 0) {
            min = a;
            pos_min = i;
        }
//        printf("%d %lld\n", i, a);
    }

    lseek(fd, pos_min, SEEK_SET);

    ll a = -min;
    unsigned char *buf = (unsigned char*) &a;

    ssize_t count;
    int tmp = 0;
    while ((count = write(fd, buf + tmp, NUM_SIZE - tmp)) > 0){
        tmp += count;
        if (tmp == 4){
            break;
        }
    }
    return 0;
}
