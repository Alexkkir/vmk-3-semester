#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

#define IN p_pos
#define OUT (p_pos + 1) % 2

#define IS_EMPTY(read) finished |= (read > 0 ? 0 : 1)

enum { NAME_LEN = 100, ACCESS_RIGHTS = 0777, BUFF_SIZE = 1, };

int n;

void
func(int in1, int in2, int out)
{
    int val1 = 0, val2 = 0;
    int finished = 0;

    int first_iter = 1;
    while (!finished) {
        if (val1 == val2) {
            if (!first_iter) {
                write(out, &val1, sizeof(val1));
            } else
                first_iter = 0;
            IS_EMPTY(read(in1, &val1, sizeof(val1)));
            IS_EMPTY(read(in2, &val2, sizeof(val2)));
        } else if (val1 < val2) {
            IS_EMPTY(read(in1, &val1, sizeof(val1)));
        } else {
            IS_EMPTY(read(in2, &val2, sizeof(val2)));
        }
    }
}

int
main(int argc, char **argv)
{
//    FILE *in1, *in2, *out;
//    in1 = fopen(argv[1], "r");
//    in2 = fopen(argv[2], "r");
//
//    int p[4][2];
//    pipe(p[1]);
//    pipe(p[2]);
//    pipe(p[3]);
//
//    int x;
//    while (fscanf(in1, "%d", &x) > 0)
//        write(p[1][1], &x, sizeof(x));
//    while (fscanf(in2, "%d", &x) > 0)
//        write(p[2][1], &x, sizeof(x));
//    close(p[1][1]);
//    close(p[2][1]);
//
//    func(p[1][0], p[2][0], p[3][1]);
//    close(p[3][1]);
//
//    while (read(p[3][0], &x, sizeof(x)) > 0)
//        printf("%d\n", x);
//    return 0;

    n = argc - 1;

    int max_width = 1, height = 0; // consider work of algorithm as a tree
    while (max_width < n) {
        max_width <<= 1;
        height++;
    }

//    printf("== %d %d ==\n", max_width, height);

    int p[2][n + 1][2], p_pos = 0;
    for (int i = 0; i < n; i++) {
        pipe(p[IN][i]);
        FILE *f = fopen(argv[1 + i], "r");
        int x;
        while(fscanf(f, "%d", &x) > 0) {
            write(p[IN][i][1], &x, sizeof(x));
        }
        fclose(f);
        close(p[IN][i][1]);
    }
//    p_pos = (p_pos + 1) % 2;

    int max_count = n;
    int n_processes = 0;
    for (int seg = 2, level = 1; level <= height; level++, seg *= 2) {
        const int delta = seg / 2;
        int i, count;
//        printf("max_count: %d\n", max_count);
        for (i = 0, count = 0; count <= max_count - 2; i += seg, count += 2) {
            pipe(p[OUT][i]);
            int in_1, in_2, out;
            in_1 = p[IN][i][0];
            in_2 = p[IN][i + delta][0];
            out = p[OUT][i][1];
//            printf("%d %d --> %d\n", i, i + delta, i);
            if (!fork()) {
                func(in_1, in_2, out);
                _exit(0);
            }
            n_processes++;
            close(in_1);
            close(in_2);
            close(out);
        }
        if (max_count % 2) {
            pipe(p[OUT][i]);
            int in, out;
            in = p[IN][i][0];
            out = p[OUT][i][1];
//            printf("%d --> %d\n", i, i);
            char buff[BUFF_SIZE];
            int size = 0;
            while((size = read(in, buff, sizeof(buff))) > 0) { // copying
                write(out, buff, size);
            }
            close(in);
            close(out);
        }
        max_count += max_count % 2;
        max_count /= 2;
        p_pos = (p_pos + 1) % 2;
    }

//    printf("Answer:\n");
    int x;
    while (read(p[IN][0][0], &x, sizeof(x)) > 0)
        printf("%d\n", x);

    return 0;
}
