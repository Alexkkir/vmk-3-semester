#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <limits.h>

enum { PERMS = 0700, PATH_LEN = 1000, CHUNK = 1000, };

const char prog_template[] =
        "#!\n"
        "import os\n"
        "print(%s)\n"
        "os.unlink(\"%s\")\n"
        ;

int main(int argc, char **argv) {
    char *dir, default_dir[] = "/tmp";
    dir = getenv("XDG_RUNTIME_DIR");
    if (!dir) {
        dir = getenv("TMPDIR");
        if (!dir) {
            dir = default_dir;
        }
    }


    char path[PATH_LEN] = {0};
    sprintf(path, "%s/pyprog_%ld_%d_%d", dir, time(NULL), getpid(), rand());

    int str_len = 0;
    for (int i = 1; i < argc; i++)
        str_len += strlen(argv[i]) + 1;
    str_len += 1;

    char *formula = calloc(str_len, sizeof(char));
    int pos = 0;
    for (int i = 1; i < argc; i++) {
        sprintf(formula + pos, i < argc - 1 ? "%s*" : "%s", argv[i]);
        pos += strlen(argv[i]) + 1;
    }

    char *prog = calloc(str_len + sizeof(prog_template) + sizeof(path) + 1, sizeof(char));
    sprintf(prog, prog_template, formula, path);

    int prog_len = strlen(prog);
    int fd = open(path, O_RDWR | O_CREAT | O_TRUNC, PERMS);
    int total = 0, tmp;
    while((tmp = write(fd, prog + total, prog_len - total)) > 0) {
        total += tmp;
        if (total == prog_len) break;
    }
    close(fd);

    execlp("python3", "python3", path, NULL);
    unlink(path);
    return 0;
}
