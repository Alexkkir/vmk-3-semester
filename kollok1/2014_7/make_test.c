#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
//#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <fcntl.h>
#include <string.h>

int main() {
    int nums[] = {9, 8, 7, 6, 5, 4, 3, 2, 1};
    int n = sizeof(nums) / sizeof(nums[0]);
    char name[] = "test";
    int fd = open(name, O_RDWR | O_CREAT | O_TRUNC, 0777);
    for (int i = 0; i < n; i++) {
        int x = nums[i];
        write(fd, &x, sizeof(x));
    }
    close(fd);
    return 0;
}

