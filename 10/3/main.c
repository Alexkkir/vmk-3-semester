#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

int run(const char *str) {
    int pid = 0;
    if ((pid = fork()) == 0) {
        execlp(str, str, NULL);
        _exit(1);
    } else if (pid == -1) {
        return 0;
    } else {
        int st = 0;
        wait(&st);
        if (WIFEXITED(st) != 0 && WEXITSTATUS(st) == 0) return 1; else return 0;
    }
}

int main(int argc, char **argv) {
    int ans = ((!run(argv[1]) ? run(argv[2]) : 1) ? run(argv[3]) : 0);
    return !ans;
}
