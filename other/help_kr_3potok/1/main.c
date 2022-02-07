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
    int fd_in = open(argv[1], O_RDWR);
    int n = lseek(fd_in, 0, SEEK_END);

    int fd = open(argv[2], O_RDWR | O_CREAT, 0777);

    if (fd_in == -1 || fd == -1) {
        return 0;
    }

    if (n % sizeof(int) != 0) {
        return 0;
    }
    n /= sizeof(int);


    char buffer[1000];
    int count;
    lseek(fd_in, 0, SEEK_SET);
    while ((count = read(fd_in, buffer, sizeof(buffer))) > 0) {
        write(fd, buffer, count);
    }


    int x1, x2;
    int ok = 0;
    while (!ok) {
        ok = 1;
        for (int i = 0; i < n - 1; i++) {
            int i1 = i, i2 = i + 1;

            lseek(fd, i1 * sizeof(int), SEEK_SET);
            read(fd, &x1, sizeof(x1));

            lseek(fd, i2 * sizeof(int), SEEK_SET);
            read(fd, &x2, sizeof(x2));

            if (x1 > x2) {
                ok = 0;

                lseek(fd, i1 * sizeof(int), SEEK_SET);
                write(fd, &x2, sizeof(x2));

                lseek(fd, i2 * sizeof(int), SEEK_SET);
                write(fd, &x1, sizeof(x1));

                break;
            }
        }
    }
    return 0;
}
