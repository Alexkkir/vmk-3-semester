#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    int p[2], file1, file2;
    pipe(p);
    int st = 0;

    // запускаем группу
    if (!fork()) {
        // запускаем 1 команду
        close(p[0]);
        if (!fork()) {
            dup2(p[1], 1);
            close(p[1]);

            file1 = open(argv[4], O_RDONLY);
            if (file1 == -1) {
                _exit(1);
            }
            dup2(file1, 0);
            close(file1);

            execlp(argv[1], argv[1], NULL);
            _exit(1);
        }
        wait(&st);
        if (WIFEXITED(st) != 0 && WEXITSTATUS(st) == 0) st = 1; else st = 0; // если плохо, то выходим
        if (st == 0) {
            close(p[1]);
            _exit(1);
        }

        // запускаем 2 команду
        if (!fork()) {
            dup2(p[1], 1);
            close(p[1]);
            execlp(argv[2], argv[2], NULL);
            _exit(0);
        }
        close(p[1]);
        wait(NULL);
        _exit(0);
    }

    // и наконец 3
    if (!fork()) {
        close(p[1]);
        dup2(p[0], 0);
        close(p[0]);

        file2 = open(argv[5], O_CREAT | O_WRONLY | O_APPEND, 0777);
        if (file2 == -1) {
            _exit(1);
        }
        dup2(file2, 1);
        close(file2);

        execlp(argv[3], argv[3], NULL);
        _exit(1);
    }

    close(p[1]);
    close(p[0]);
    wait(NULL);
    wait(NULL);
    return 0;
}
