#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char **argv) {
    int fd = open(argv[1], O_RDWR);
    int n = lseek(fd, 0, SEEK_END) / 4;
    int ok = 0, x_prev;
    while (!ok) {
        ok = 1;
        lseek(fd, 0, SEEK_SET);
        read(fd, &x_prev, sizeof(x_prev));
        for (int i = 1; i < n; i++) {
            int x;
            read(fd, &x, sizeof(x));
            if (x_prev > x) {
                ok = 0;
                lseek(fd, -2 * sizeof(x), SEEK_CUR);
                write(fd, &x, sizeof(x_prev));
                write(fd, &x_prev, sizeof(x_prev));
                break;
            } else {
                x_prev = x;
            }
        }
    }
    close(fd);
    return 0;
}
