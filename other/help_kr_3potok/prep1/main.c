#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>

int
main(int agrc, char **argv)
{
    int fd = open(argv[1], O_RDWR);
    int n = lseek(fd, 0, SEEK_END) / sizeof(int);
//    printf("n: %d\n", n);

    for (int i1 = 0; i1 < n / 2; i1++) {
        int x1, x2;
        int i2 = n - i1 - 1;

        lseek(fd, i1 * sizeof(int), SEEK_SET);
        read(fd, &x1, sizeof(x1));

        lseek(fd, i2 * sizeof(int), SEEK_SET);
        read(fd, &x2, sizeof(x2));

//        printf("x1, x2: %d %d\n", x1, x2);

        lseek(fd, i1 * sizeof(int), SEEK_SET);
        write(fd, &x2, sizeof(x2));

        lseek(fd, i2 * sizeof(int), SEEK_SET);
        write(fd, &x1, sizeof(x1));
    }

    close(fd);
    return 0;
}
