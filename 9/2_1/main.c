#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int
main(int argc, char *argv[])
{
    pid_t pid;
    if((pid = fork()) == 0) {
        if((pid = fork()) == 0) {
            printf("3 ");
        } else if(pid > 0) {
            wait(0);
            printf("2 ");
        }
    } else if(pid > 0) {
        wait(0);
        printf("1\n");
    }
}