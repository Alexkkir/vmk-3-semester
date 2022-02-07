#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>

volatile sig_atomic_t counter = 0;
volatile sig_atomic_t ok = 0;

void handl(int s) {
    if (counter < 5) {
        printf("%d\n", counter);
        fflush(stdout);
        counter++;
    } else {
        ok = 1;
    }
}

int main() {
    sigaction(SIGHUP, &(struct sigaction) {.sa_handler = handl, .sa_flags = SA_RESTART}, NULL);
    printf("%d\n", getpid());
    fflush(stdout);

    ok = 0;
    while(!ok) {
        pause();
    }
    return 0;
}
