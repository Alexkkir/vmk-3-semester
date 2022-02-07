#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>

enum { LEN = 4, };

int
main(int argc, char *argv[]) {
//    argv[1] = "C:\\Users\\1\\Documents\\EVM\\6_ontest\\2_2\\cmake-build-debug";
    DIR *d = opendir(argv[1]); // открываем дирректорию
    struct dirent *dd;

    if (!d) {
        exit(1); // не открылось...
    }
    long long count = 0;
    const char sample[] = ".exe";
    while ((dd = readdir(d))) { // читаем очередной элемент из дирректории
        if (!strcmp(dd->d_name, ".") || !strcmp(dd->d_name, "..")) continue;
        char path[PATH_MAX];
        snprintf(path, sizeof(path), "%s/%s", argv[1], dd->d_name);
        struct stat stb;

        if (access(path, X_OK) == 0 && // проверяем факт сущестовавания
            stat(path, &stb) == 0 && // проверяем (снова) факт существования + еще какие-то проверки, возможно
            S_ISREG(stb.st_mode) && // регулярный?
            !strncmp(&dd->d_name[strlen(dd->d_name) - LEN], sample, LEN)) { // окначивается на /exe?
            count += 1;
        }
    }

    closedir(d);
    printf("%lld\n", count);
}

