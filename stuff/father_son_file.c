#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/signal.h>

int main() {
    int fd = open("1.txt", O_RDWR);
    char b[100] = {0};
    char c;
    pid_t pid;
    if (!(pid = fork())) {
        read(fd, &c, 1);
        printf("%c", c); fflush(stdout);
        kill(getppid(), SIGUSR1);
        pause();
        read(fd, &c, 1);
        printf("%c", c); fflush(stdout);
        kill(getppid(), SIGUSR1);
        _exit(0);
    } else {
        pause();
        read(fd, &c, 1);
        printf("%c", c); fflush(stdout);
        kill(pid, SIGUSR1);
        _exit(0);
    }
}
