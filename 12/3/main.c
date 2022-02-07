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


typedef void (*sighandler_t)(int);

void handl(int s);

void handl(int s) {

}

int main(int argc, char **argv) {
    int counter = 0;


    int N = strtol(argv[1], NULL, 10);
    for (int i = 2; i < N + 2; i++) {
        if (!fork()) {
            FILE *fp = fopen(argv[i], "r");
            char name[PATH_MAX + 2];

            fgets(name, PATH_MAX + 2, fp);

            int len = strlen(name);
            name[len - 1] = 0;
            fflush(stdout);
            execlp(name, name, NULL);
            _exit(1);
        }
    }

    int st;
    for (int i = 0; i < N; i++){
        wait(&st);
        if (WIFEXITED(st) != 0 && WEXITSTATUS(st) == 0) st = 1; else st = 0;
        counter += st;
    }

    for (int i = N + 2; i < argc; i++) {
        if (!fork()) {
            FILE *fp = fopen(argv[i], "r");
            char name[PATH_MAX + 2];

            fgets(name, PATH_MAX + 2, fp);

            int len = strlen(name);
            name[len - 1] = 0;
            fflush(stdout);
            execlp(name, name, NULL);
            _exit(1);
        }
        int st;
        wait(&st);
        if (WIFEXITED(st) != 0 && WEXITSTATUS(st) == 0) st = 1; else st = 0;
        counter += st;
    }

    printf("%d\n", counter);
    return 0;
}

