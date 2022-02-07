#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/wait.h>

enum { PATH_LEN = 200, PERMS = 0700, NAME_LEN = 100, };


const char template[] =
        "#include <math.h>\n"
        "\n"
        "double fun(double x) {\n"
        "    return %s;\n"
        "}";

int
main(int argc, char **argv)
{
    char program[1000];
    sprintf(program, template, argv[4]);

    char *dir, default_dir[] = "/tmp";
    dir = getenv("XDG_RUNTIME_DIR");
    if (!dir) {
        dir = getenv("TMPDIR");
        if (!dir) {
            dir = default_dir;
        }
    }

    char path_c[PATH_LEN] = {0};
    char path_lib[PATH_LEN] = {0};
    char name[NAME_LEN] = {0};
    srand(time(NULL));
    sprintf(name, "cprog_%u", (unsigned) time(NULL) + (unsigned) getpid() + (unsigned) rand());
    sprintf(path_c, "%s/%s.c", dir, name);
    sprintf(path_lib, "%s/%s.so", dir, name);

    int fd = open(path_c, O_WRONLY | O_CREAT | O_TRUNC, PERMS);
    int count = 0, tmp;
    const int prog_len = strlen(program);
    while (count != prog_len && (tmp = write(fd, program + count, prog_len - count)) > 0) {
        count += tmp;
    }

    if (fork() == 0) {
        execlp("gcc", "gcc", "-shared", "-o", path_lib, "-fPIC", path_c, "-lm", NULL);
    }
    wait(NULL);

    void *handle;
    handle = dlopen(path_lib, RTLD_LAZY);
    if (handle == NULL) {
        _exit(1);
    }
    double (*fun)(double x) = dlsym(handle, "fun");

    double left = strtod(argv[1], NULL);
    double right = strtod(argv[2], NULL);
    int n = strtol(argv[3], NULL, 10);

    double dx = (right - left) / n;
    double integral = 0;
    for (int i = 0; i < n; i++) {
        double x = left + dx * i;
        integral += fun(x) * dx;
    }

    printf("%.10g\n", integral);
    unlink(path_c);
    unlink(path_lib);

    return 0;
}
