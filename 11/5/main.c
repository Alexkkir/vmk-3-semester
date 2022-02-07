#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

enum { RADIX = 10, };

void
func(FILE * in, FILE * out, int fd_in, int fd_out, int lim, int user_pid)
{
    while (1) {
        fflush(stdout);
        int x;
        if (fscanf(in, "%d", &x) != 1) {
            close(fd_in);
            close(fd_out);
            return;
        }
        if (x != lim - 1) {
            ++x;
            printf("%d %d\n", user_pid, x);
            fflush(stdout);
            fprintf(out, "%d\n", x);
            fflush(out);
        } else {
            close(fd_in);
            close(fd_out);
            return;
        }
    }
}

int
main(int argc, char **argv)
{
    int lim = strtol(argv[1], NULL, RADIX);
    int pipe_1[2], pipe_2[2];
    pipe(pipe_1), pipe(pipe_2);
    FILE *in1, *in2, *out1, *out2;
    in1 = fdopen(pipe_1[0], "r");
    in2 = fdopen(pipe_2[0], "r");
    out1 = fdopen(pipe_1[1], "w");
    out2 = fdopen(pipe_2[1], "w");
    setbuf(in1, NULL);
    setbuf(in2, NULL);

    fprintf(out1, "0\n");
    fflush(out1);
    if(!fork()) {
        close(pipe_1[1]);
        close(pipe_2[0]);
        func(in1, out2, pipe_1[0], pipe_2[1], lim, 1);
        close(pipe_1[0]);
        close(pipe_2[1]);
        _exit(1);
    }
    if(!fork()) {
        close(pipe_1[0]);
        close(pipe_2[1]);
        func(in2, out1, pipe_2[0], pipe_1[1], lim, 2);
        close(pipe_1[1]);
        close(pipe_2[0]);
        _exit(1);
    }

    close(pipe_1[0]);
    close(pipe_1[1]);
    close(pipe_2[0]);
    close(pipe_2[1]);
    wait(NULL);
    wait(NULL);
    printf("Done\n");
    return 0;
}