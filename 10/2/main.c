#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

int mysys(const char *str) {
    int pid = 0;
    if ((pid = fork()) == 0) {
        execl("/bin/sh", "sh", "-c", str, NULL);
        _exit(127);
    } else if (pid == -1) {
        return -1;
    } else {
        int st;
        waitpid(pid, &st, 0);
        if (WIFEXITED(st) != 0) {
            return WEXITSTATUS(st);
        } else if (WIFSIGNALED(st) != 0) {
            return 128 + WTERMSIG(st);
        } else if (WIFSTOPPED(st) != 0) {
            return 128 + WSTOPSIG(st);
        }
    }
    return 0;
}

