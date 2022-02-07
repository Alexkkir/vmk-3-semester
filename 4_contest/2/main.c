#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    unsigned a[10000];
    int pos = 0;
    while(scanf("%d", &a[pos]) > 0) {
        pos++;
    }
    int n = pos;

    int fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, 0600);
    for (int i = 0; i < n; i++) {
        unsigned char b1, b2, b3, b4;
        b4 = a[i] & 0xFF;
        b3 = (a[i] >> 8) & 0xF;
        b2 = (a[i] >> 12) & 0xFF;
        b1 = (a[i] >> 20) & 0xF;

        unsigned char buf[] = {b1, b2, b3, b4};

        ssize_t count;
        int tmp = 0;
        while ((count = write(fd, buf + tmp, 4 - tmp)) > 0){
            tmp += count;
            if (tmp == 4){
                break;
            }
        }
    }

    close(fd);

    return 0;
}
