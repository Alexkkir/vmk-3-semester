#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    int pid = fork();
    if (pid == 0) {
        int in = open(argv[2], O_RDONLY, 0);
        int out = open(argv[3], O_WRONLY | O_CREAT | O_APPEND, 0660); // ??? trunc
        int err = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0660);

        if (in == -1 || out == -1 || err == -1) _exit(42);

        if (dup2(in, 0) == -1) _exit(42);
        if (dup2(out, 1) == -1) _exit(42);
        if (dup2(err, 2) == -1) _exit(42);

        if (close(in) == -1) _exit(42);
        if (close(out) == -1) _exit(42);
        if (close(err) == -1) _exit(42);

        if (execlp(argv[1], argv[1], NULL) == -1) _exit(42);
        _exit(42);
    } else {
        int st = 0;
        wait(&st);
//        st = WEXITSTATUS(st);
        printf("%d\n", st   );
        return 0;
    }
}