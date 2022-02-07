#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>

int
main()
{
    struct stat buff;
    int fd = open("1.txt", O_RDONLY);

    fstat(fd, &buff);
    printf("access: %d\n", access("1.txt", R_OK));
    return 0;
}
