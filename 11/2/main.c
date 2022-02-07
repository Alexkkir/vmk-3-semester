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
    // отец
    int fd[2];
    pipe(fd);
    if (fork() == 0) {
        // сын

        if (fork() == 0) {
            // внук
            close(fd[1]);
            int x;
            long long sum = 0;
            while(read(fd[0], &x, sizeof(x)) != 0) {
                sum += x;
            }
            close(fd[0]);
            printf("%lld\n", sum);
            return 0;
        }
        close(fd[0]);
        close(fd[1]);
        wait(NULL);
        return 0;
    }
    close(fd[0]);
    int x;
    while(scanf("%d", &x) > 0) {
        write(fd[1], &x, sizeof(x));
    }
    close(fd[1]);
    wait(NULL);
    return 0;
}
