#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>

int total = 0;

enum {
    PLUS,
    MULT
} op = PLUS;

typedef void (*sighandler_t)(int);
void handl(int s);

void handl(int s) {
    signal(SIGINT, handl);
    signal(SIGQUIT, handl);

    if (s == SIGINT) {
        op = PLUS;
    } else if (s == SIGQUIT) {
        op = MULT;
    }
}

int main() {
    /////

    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    printf("%d\n", getpid());

    signal(SIGINT, handl);
    signal(SIGQUIT, handl);

    int x;
    while (scanf("%d", &x) == 1) {
        if (op == PLUS) {
            total = (int) ((unsigned) total + (unsigned) x);
        } else {
            total = (int) ((unsigned) total * (unsigned) x);
        }
        printf("%d\n", total);
        fflush(stdout);
    }
    return 0;
}
