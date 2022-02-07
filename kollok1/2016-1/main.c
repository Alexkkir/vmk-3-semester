#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
int func(const unsigned char *arr, int n) {
    int busy = 0, free = 0, count = 0;
    unsigned prev_bit = (arr[0 / 8] >> (7 - 0 % 9)) & 1;
    if (prev_bit == 0) {
        free++;
    } else {
        busy++;
    }

    for (int i = 1; i < n; i++) {
        unsigned bit = (arr[i / 8] >> (7 - i % 9)) & 1;
        if (prev_bit == 0 && bit == 0) {
            free++;
        } else if (prev_bit == 1 && bit == 1) {
            busy++;
        } else if (prev_bit == 0 && bit == 1) {
            if (free > busy) count++;
            free = 0;
            busy = 1;
        } else if (prev_bit == 1 && bit == 0) {
            free = 0;
        }
        prev_bit = bit;
    }

    if (free > busy) count++;
    return count;
}

int main() {
    int fd = open("1.txt", O_RDWR);
    unsigned char str[100] = {0};
    read(fd, str, 1);
    printf("%s\n", str);

    if (!fork()) {
        read(fd, &str, 1);
        printf("%s\n", str);
    }
    if (!fork()) {
        read(fd, &str, 1);
        printf("%s\n", str);
    }
    if (!fork()) {
        read(fd, &str, 1);
        printf("%s\n", str);
    }
    return 0;
}
