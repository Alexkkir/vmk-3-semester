#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

typedef long long ll;

enum {
    kb = 1024,
};

int
main(int argc, char **argv)
{
    long long sum_size = 0;
    for (int nf = 1; nf < argc; nf++) {
        char *name = argv[nf];
        struct stat buf;
        lstat(name, &buf);
        if (
                (lstat(name, &buf) != -1) &&
                (buf.st_size % kb == 0) &&
                (S_ISREG(buf.st_mode) == 1) &&
                (S_ISLNK(buf.st_mode) != 1) &&
                (buf.st_nlink == 1))
        {
            sum_size += buf.st_size;
        }
    }
    printf("%lld\n", sum_size);

    return 0;
}
