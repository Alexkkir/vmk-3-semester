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

enum { PATH_LEN = 200, PERMS = 0700, NAME_LEN = 100, NUM_LEN = 15, };

const char prog_template[] =
        "#include <stdio.h>\n"
        "enum Result {\n"
        "    reject,\n"
        "    summon,\n"
        "    disqualify,\n"
        "};\n"
        "\n"
        "int main(int argc, char **argv) {\n"
        "    int x;\n"
        "    sscanf(argv[1], \"%%d\", &x);\n"
        "    enum Result res = (%s);\n"
        "    switch(res) {\n"
        "        case reject:\n"
        "            printf(\"reject\\n\");\n"
        "            break;\n"
        "        case summon:\n"
        "            printf(\"summon\\n\");\n"
        "            break;\n"
        "        case disqualify:\n"
        "            printf(\"disqualify\\n\");\n"
        "            break;\n"
        "        default:\n"
        "            break;\n"
        "    }\n"
        "    return 0;\n"
        "}\n"
        ;

int main(int argc, char **argv) {
    // choosing dir
    char *dir, default_dir[] = "/tmp";
    dir = getenv("XDG_RUNTIME_DIR");
    if (!dir) {
        dir = getenv("TMPDIR");
        if (!dir) {
            dir = default_dir;
        }
    }

    // creating random name for file
    char path_c[PATH_LEN] = {0};
    char path_exec[PATH_LEN] = {0};
    char name[NAME_LEN] = {0};
    srand(time(NULL));
    sprintf(name, "cprog_%u", (unsigned) time(NULL) * (unsigned) getpid() * (unsigned) rand());
    sprintf(path_c, "%s/%s.c", dir, name);
    sprintf(path_exec, "%s/%s", dir, name);

    // writing program to file
    char *prog = calloc(strlen(prog_template) + strlen(argv[1]) + 1, sizeof(char));
    sprintf(prog, prog_template, argv[1]);
    int fd = open(path_c, O_WRONLY | O_CREAT | O_TRUNC, PERMS);
    int count = 0, tmp;
    const int prog_len = strlen(prog);
    while(count != prog_len && (tmp = write(fd, prog + count, prog_len - count)) > 0) {
        count += tmp;
    }

    // handling user's input
    if (fork() == 0) {
        execlp("gcc", "gcc", path_c, "-o", path_exec, NULL);
    }
    wait(NULL);
    char x[NUM_LEN] = {0};
    while(scanf("%s", x) > 0) {
        if (fork() == 0) {
            execl(path_exec, path_exec, x, NULL);
        }
        wait(NULL);
    }

    // preparing to terminate process
    unlink(path_c);
    unlink(path_exec);
    free(prog);
    return 0;
}
