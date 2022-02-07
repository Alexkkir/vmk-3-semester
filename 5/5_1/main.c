#include <stdio.h>
#include <fcntl.h> // O_RDONLY
#include <unistd.h> // open, read

enum { SIZE = 2 };

int
main(int argc, char **argv)
{
    int fd = open(argv[1], O_RDONLY);
    unsigned char buff[SIZE], buff_inv[SIZE];
    unsigned short min = ~0, x, found = 0;
    while (read(fd, buff, SIZE) > 0) {
        for (int i = 0; i < SIZE; i++) {
            buff_inv[i] = buff[SIZE - i - 1];
        }
        x = *((unsigned short*) buff_inv);
        if (!(x & 1)) {
            found = 1;
            min = x < min ? x : min;
        }
    }
    if (found)
        printf("%u\n", min);
    return 0;
}
