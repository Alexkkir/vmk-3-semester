#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>

int main() {
    if (!fork()) {
        int p[2];
        pipe(p);
        if (!fork()) {
            close(p[0]);
            pid_t pids[2] = {getppid(), getpid()};
            write(p[1], pids, sizeof(pids));
            close(p[1]);
            _exit(0);
        }
        if (!fork()) {
            close(p[1]);
            pid_t pids[2];
            read(p[0], pids, sizeof(pids));
            printf("%d %d\n", p[0], p[1]);
            fflush(stdout);
            close(p[1]);
            _exit(0);
        }
        close(p[0]);
        close(p[1]);
        wait(NULL);
        wait(NULL);
    }
    wait(NULL);
    return 0;
}
