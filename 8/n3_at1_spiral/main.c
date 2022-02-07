#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <fcntl.h>
//#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define arr(i, j) arr[(i) * m + (j)]

enum { N_DIR = 4, RADIX = 10 };
enum Dir { RIGHT, DOWN, LEFT, UP };

static void next(int i, int j, enum Dir dir, int *p_i_2, int *p_j_2) {
    if (dir == RIGHT) {
        *p_i_2 = i;
        *p_j_2 = j + 1;
    } else if (dir == DOWN) {
        *p_i_2 = i + 1;
        *p_j_2 = j;
    } else if (dir == LEFT) {
        *p_i_2 = i;
        *p_j_2 = j - 1;
    } else if (dir == UP) {
        *p_i_2 = i - 1;
        *p_j_2 = j;
    }
}

static int empty(int *arr, int n, int m, int i, int j) {
    if (0 <= i && i < n && 0 <= j && j < m) {
        return arr(i, j) == 0 ? 1 : 0;
    } else {
        return 0;
    }
    return 0;
}

static void update_dir(enum Dir *p_dir) {
    *p_dir = (*p_dir + 1) % N_DIR;
}

int main(int argc, char **argv) {
    int n, m;
    n = strtol(argv[2], NULL, RADIX);
    m = strtol(argv[3], NULL, RADIX);

    int fd = open(argv[1], O_RDWR | O_CREAT);
    ftruncate(fd, n * m * sizeof(int));
//    void *vptr = mmap(NULL, n * m * sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int *arr = calloc(n * m, sizeof(int));
    memset(arr, 0, n * m * sizeof(int));
    enum Dir dir;
    int i = 0, j = 0;
    int i_2 = 0, j_2 = 0;

    dir = RIGHT;
    arr = 0;

    for (int step = 1; step <= n * m; step++) {
        arr(i, j) = step;
        next(i, j, dir, &i_2, &j_2);
        if (empty(arr, n, m, i_2, j_2)) {
            i = i_2, j = j_2;
        } else {
            update_dir(&dir);
            next(i, j, dir, &i_2, &j_2);
            i = i_2, j = j_2;
        }
    }

    close(fd);
    return 0;
}
