#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

enum {
    RADIX = 10,
};

void
handler(int signal);

void
handler_for_pid(int signal);

void
func(void);

int p[2];
int lim = -1, my_pid = -1, other_pid = -1, type = -1;
volatile sig_atomic_t working = 1;
FILE *in, *out;
int fd_in, fd_out;

void
handler(int s)
{
    signal(SIGUSR1, handler);
    func();
    kill(other_pid, SIGUSR1);
}


volatile sig_atomic_t pid_1_written = 0;

void
handler_for_pid(int s)
{
    signal(SIGUSR2, handler_for_pid);
    pid_1_written = 1;
}

void
func(void)
{
    int x;
    fscanf(in, "%d", &x);
    if (x != lim - 1) {
        ++x;
        printf("%d %d\n", type, x);
        fflush(stdout);
        fprintf(out, "%d\n", x);
        fflush(out);
    } else {
        working = 0;
        fprintf(out, "%d\n", x);
        fflush(out);
        close(p[0]);
        close(p[1]);
        return;
    }
}


int
main(int argc, char **argv)
{
    lim = strtol(argv[1], NULL, RADIX);
    pipe(p);
    FILE *in_1, *out;
    in = fdopen(p[0], "r");
    out = fdopen(p[1], "w");
    setbuf(in, NULL);
    setbuf(out, NULL);

    signal(SIGUSR1, handler);

    int pid;
    if (!fork()) {
        signal(SIGUSR2, handler_for_pid);
        my_pid = getpid();
        type = 2;
        fprintf(out, "%d\n", my_pid);
        fflush(out);
        while (!pid_1_written) pause();
        fscanf(in, "%d", &other_pid);

        fprintf(out, "0\n");
        fflush(out);

        kill(other_pid, SIGUSR1);

        while (working) pause();

        _exit(0);
    }
    if (!fork()) {
        my_pid = getpid();
        type = 1;
        fscanf(in, "%d", &other_pid);
        kill(other_pid, SIGUSR2);
        fprintf(out, "%d\n", my_pid);
        fflush(out);

        while (working) pause();

        _exit(0);
    }
    close(p[0]);
    close(p[1]);
    wait(NULL);
    wait(NULL);
    printf("Done\n");
    return 0;
}