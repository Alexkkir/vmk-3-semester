#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {

    int fd[2];
    pipe(fd);
    struct tm *pt;
    // отец
    if (fork() == 0) {
        // сын
        if (fork() == 0) {
            // внук
            if (fork() == 0) {
                // правнук
                time_t t = time(NULL);
                close(fd[0]);
                write(fd[1], &t, sizeof(t));
                write(fd[1], &t, sizeof(t));
                write(fd[1], &t, sizeof(t));
                close(fd[1]);
                return 0;
            }
            wait(NULL);
            time_t t;
            close(fd[1]);
            read(fd[0], &t, sizeof(t));
            close(fd[1]);
            pt = localtime(&t);
            printf("D:%02d\n", pt->tm_mday);
            return 0;
        }
        wait(NULL);
        time_t t;
        close(fd[1]);
        read(fd[0], &t, sizeof(t));
        close(fd[1]);
        pt = localtime(&t);
        printf("M:%02d\n", pt->tm_mon + 1);
        return 0;
    }
    wait(NULL);
    time_t t;
    close(fd[1]);
    read(fd[0], &t, sizeof(t));
    close(fd[1]);
    pt = localtime(&t);
    printf("Y:%04d\n", pt->tm_year + 1900);
    return 0;
}
