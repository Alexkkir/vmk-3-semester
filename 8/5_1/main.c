#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>

enum { CHUNK = 1, };

int comp(void const *i, void const *j) {
    return strcasecmp(*(const char **) i, *(const char **) j);
}

void
fun(char *path_in) {
    DIR *dir = opendir(path_in);
    if (!dir) return;
    struct dirent *elem;
    int len = strlen(path_in);

    char *path = calloc(PATH_MAX, sizeof(char));

    char **names = malloc(CHUNK * sizeof(char *));
    int pos = 0, size = CHUNK;
    while ((elem = readdir(dir))) {
        if (pos >= size) {
            size += CHUNK;
            names = realloc(names, size * sizeof(char *));
        }

        snprintf(path, PATH_MAX, "%s/%s", path_in, elem->d_name);

        if (!strcmp(elem->d_name, ".") || !strcmp(elem->d_name, "..")) continue;
        DIR *test_d = opendir(path);
        if (!test_d) continue; else closedir(test_d);
        struct stat stb;
        if (lstat(path, &stb) < 0) continue;
        if (!S_ISDIR(stb.st_mode)) continue;

        names[pos] = calloc(strlen(elem->d_name) + 1, sizeof(char));
        strcpy(names[pos], elem->d_name);
        pos++;
    }
    closedir(dir);
    qsort(&names[0], pos, sizeof(char *), comp);
//    for (int i = 0; i < pos; i++) printf("\t> %s\n", names[i]);

    for (int i = 0; i < pos; i++) {
        char *name = &*names[i];
        sprintf(path + len, "/%s", name);

        printf("cd %s\n", name);
        fun(path);
        printf("cd ..\n");
    }
    for (int i = 0; i < pos; i++) {
        free(names[i]);
    }
    free(names);
}

int
main(int argc, char **argv) {
    char path[PATH_MAX] = {0};
    strcpy(path, argv[1]);
    fun(path);
    return 0;
}