#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    int n;
    int count = 0, flag = 0;
    scanf("%d", &n);

    while (!flag && count < n) {
        printf(count < n - 1 ? "%d " : "%d\n", count + 1);
        fflush(stdout);
        flag = 1;
        count++;
        if (fork() == 0) {
            // child
            flag = 0;
        } else {
            // father
            wait(0);
        }
    }

    return 0;
}
