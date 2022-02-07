#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>
#include <sys/wait.h>

int
main(int argc, char **argv)
{
    char dir[] = "/tmp"; // new file
    char path[PATH_MAX];
    srand(time(NULL));
    sprintf(path, "%s/%d_%d_%d", dir, rand(), rand(), rand());
    int tmpf = open(path, O_RDWR | O_CREAT, 0777);

    int fd = open(argv[1], O_RDONLY);

    if (!fork()) {
        dup2(fd, 0);
        dup2(tmpf, 1);
        execlp(argv[2], argv[2], NULL);
        _exit(1);
    }

    int st;
    wait(&st);

    char buffer[4096];
    int count;

    if (WIFEXITED(st) && WEXITSTATUS(st) == 0) {
        lseek(tmpf, 0, SEEK_SET);
        close(fd);
        fd = open(argv[1], O_RDWR | O_TRUNC);
        while ((count = read(tmpf, buffer, sizeof(buffer))) > 0) {
            write(fd, buffer, count);
        }
    }

    unlink(path);
    close(fd);
    return 0;
}
