#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>

int main() {
    int val = 666, pid;
    pid_t first = getpid();
    for (;;){
        if ((pid = fork()) == 0) {
            if (scanf("%d", &val) > 0) {
                continue;
            } else {
                exit(0);
            }
        } else if (pid == -1) {
            printf("-1\n");
            exit(1);
        } else {
            int st;
            wait(&st);
            st = WEXITSTATUS(st);
            if (st == 0) {
                getpid() != first ? printf("%d\n", val) : 0;
                exit(0);
            } else if (st == 1){
                getpid() != first ? exit(1) : exit(0);
            }
        }
    }
    return 0;
}
