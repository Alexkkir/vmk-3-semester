#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char **argv) {
    int p1[2], p2[2];
    pipe(p1);
    if (!fork()) {
        dup2(p1[1], 1);
        close(p1[0]);
        close(p1[1]);
        execlp(argv[1], argv[1], NULL);
    }
    close(p1[1]);
    pipe(p2);
    if (!fork()) {
        dup2(p1[0], 0);
        dup2(p2[1], 1);
        close(p1[0]);
        close(p2[0]);
        close(p2[1]);
        execlp(argv[2], argv[2], NULL);
    }
    close(p1[0]);
    close(p2[1]);
    if (!fork()) {
        dup2(p2[0], 0);
        close(p2[0]);
        execlp(argv[3], argv[3], NULL);
    }
    close(p2[0]);
    wait(NULL);
    wait(NULL);
    wait(NULL);
    return 0;
}
