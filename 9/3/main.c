#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

enum { N = 3 };

int main() {
    for (int i = 1; i <= N; ++i) {
        pid_t pid = fork();
        if (pid == 0) {
            int x;
            char buf[9] = {0};
            read(0, buf, 8);
            x = strtol(buf, 0, 10);
            printf("%d %d\n", i, x * x);
            exit(0);
        }
    }
    for (int i = 0; i < N; i++)
            wait(0);
    return 0;
}
