#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>

typedef unsigned short us;

int main(int argc, char **argv) {
    int fd = open(argv[1], O_RDONLY, 0);
    int t = 0;
    sscanf(argv[2], "%x", &t);
    struct stat stb;
    fstat(fd, &stb);
    unsigned short *map = mmap(NULL, stb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    unsigned short *table = map + t / 2;

    unsigned v_addr;
    while (scanf("%x", &v_addr) > 0) {
        unsigned short t_num = v_addr >> 9;
        unsigned short t_addr = table[t_num];
        unsigned short p_addr = (t_addr) | (v_addr & 511);
        p_addr >>= 1;
        printf("%u\n", map[p_addr]);
    }
    munmap(map, stb.st_size);
    close(fd);
    return 0;
}
