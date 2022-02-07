#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <sys/stat.h>
#include <limits.h>

int
main(int argc, char **argv)
{
    /*
     * Sum size of files, such that:
     * 1. Regular
     * 2. Owner is cur user
     * 3. First letter of name is capital
     */

    DIR *dfd = opendir(argv[1]);
    struct dirent *entry;
    int sum_size = 0;
    read(0, &sum_size, 4);
    while ((entry = readdir(dfd))) {
        if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")) // жопная схема
            continue;
        struct stat info;
        char path_to_file[PATH_MAX];
        snprintf(path_to_file, sizeof(path_to_file), "%s/%s", argv[1], entry->d_name);
        // stat, not lstat
        if (
                stat(path_to_file, &info) >= 0 &&
                S_ISREG(info.st_mode) &&
                getuid() == info.st_uid &&
                'A' <= entry->d_name[0] &&
                entry->d_name[0] <= 'Z'
                ) {
            sum_size += info.st_size;
        }
    }

    printf("%d\n", sum_size);
    return 0;
}
