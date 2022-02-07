#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>

typedef unsigned long long ull;
int
main(int argc, char **argv)
{
    int fd = open(argv[1], O_RDWR);

    char dir[] = "/tmp"; // new file
    char path[PATH_MAX];
    srand(time(NULL));
    sprintf(path, "%s/%d_%d_%d", dir, rand(), rand(), rand());

    int tmpf = open(path, O_RDWR | O_CREAT, 0777);
    ull lim;
    sscanf(argv[2], "%llu", &lim);

    ull x;
    while(read(fd, &x, sizeof(x)) > 0) { // analyzing
        if (x >= lim) {
            write(tmpf, &x, sizeof(x));
        }
    }

    close(fd); // copying
    fd = open(argv[1], O_RDWR | O_TRUNC); // clean
    unsigned char buff[4096];
    int count;
    lseek(tmpf, 0, SEEK_SET);
    while((count = read(tmpf, buff, sizeof(buff))) > 0) {
        write(fd, buff, count);
    }

    unlink(path); // delete
    close(fd);
    close(tmpf);
    return 0;
}