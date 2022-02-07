#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <limits.h>
#include <fcntl.h>
#include <unistd.h>

enum {
    NAME_LEN = 100, ACCESS_RIGHTS = 0777,
};

int n_files;

int
main(int argc, char **argv)
{
    n_files = argc - 1;
    srand(time(NULL));
    int hash = (int) (((unsigned) rand() + (unsigned) time(NULL)) >> 1);

    int width = 1, height = 0; // consider work of algorithm as a tree
    while (width < n_files) {
        width <<= 1;
        height++;
    }

    /*
     * Explanation of algorithm (1, 2, ..., 8 - indexes of files)
     * 0 1 2 3 4 5 6 7
     * 01  23  45  67
     * 0123    4567
     * 01234567
     * ===============
     * 0 1 2 3 4 5 6
     * 01  23  45  6
     * 0123    456
     * ===============
     * 0 1 2 3 4 5
     * 01  23  45
     * 0123    45
     * 012345
     */

    // Adding some files to simplify algorithm
    int new_n_files = (n_files + 1) / 2 * 2; // normalized number of files
    char **names = calloc(new_n_files, sizeof(char *));
    for (int i = 0; i < new_n_files; i++) {
        names[i] = calloc(NAME_LEN, sizeof(char));
    }

    for (int i = 0; i < n_files; i++) {
        strcpy(names[i], argv[1 + i]);
    }
    for (int i = n_files; i < new_n_files; i++) { // just one iteration
        char new_name[NAME_LEN], *orig_name = argv[n_files];
        sprintf(new_name, "%s_%d_%d", orig_name, i + 1, hash);
        strcpy(names[i], new_name);

        int fd_orig, fd_new;
        fd_orig = open(orig_name, 0);
        fd_new = open(new_name, ACCESS_RIGHTS | O_CREAT | O_TRUNC);
        char buf;
        while (read(fd_orig, &buf, sizeof(buf)) > 0) {
            write(fd_new, &buf, sizeof(buf));
        }
    }
    int old_n_files = n_files;
    n_files = new_n_files;

    for (int level = 1, interval = 2, first_iter = 1; level <= height; level++, interval *= 2) {
        for (int i = 0; i < n_files; i += interval) {
//            int fd1, fd2, out;
            char name_1[NAME_LEN], name_2[NAME_LEN], name_out[NAME_LEN];
            sprintf(name_1, "%d_%d_%d", i, i + interval / 2, hash);
            sprintf(name_2, "%d_%d_%d", i + interval / 2, i + interval, hash);
            sprintf(name_out, "%d_%d_%d", i, i + interval, hash);

            printf("%d %d %s %s %s\n", level, i, name_1, name_2, name_out);

//            fd1 = open(name_1, 0);
//            fd2 = open(name_2, 0);
//            out = open(name_out, ACCESS_RIGHTS | O_CREAT | O_TRUNC);
//            if (fd1 != -1 && fd2 != -1)
//                process(fd1, fd2, out);
//            else {
//                if (fd1 != -1)
//                    rename(name_1, name_out);
//                else
//                    rename(name_2, name_out);
//            }
//            unlink(name_1);
//            unlink(name_2);
        }
    }


    for (int i = old_n_files; i < n_files; i++) {
        unlink(names[i]);
    }
    printf("%d %d\n", width, height);
    for (int i = 0; i < n_files; i++) {
        free(names[i]);
    }
    free(names);
    return 0;
}
