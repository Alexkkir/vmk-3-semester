#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

enum { BITS = 8, };

void set_bit(int fd, int x, int val) {
    unsigned char buf[1];
    x--;

    int ret, n_byte = x / BITS, n_bit = BITS - x % BITS - 1;

    lseek(fd, n_byte, SEEK_SET);
    ret = read(fd, buf, sizeof(buf));
    if (ret == -1) {
        exit(errno);
    }

    unsigned char bit = 1 << n_bit;
    buf[0] -= buf[0] & bit;
    buf[0] = buf[0] + bit * val;

    lseek(fd, n_byte, SEEK_SET);
    write(fd, buf, sizeof(buf));
}

int main(int argc, char **argv) {
    int fd = open("C:\\Users\\1\\Documents\\EVM\\7_contest\\4_1\\cmake-build-debug\\test1.bin", O_RDWR);
    int len = lseek(fd, 0, SEEK_END);

    int x, abs_x, val;

    int arr[] = {1, 16, -2, -15};
    for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); i++) {
        x = arr[i];
        abs_x = x > 0 ? x : -x;
        val = x > 0 ? 1 : 0;
        if (0 < abs_x && abs_x <= len * BITS)
            set_bit(fd, x, val);
    }

    close(fd);
    return 0;
}
