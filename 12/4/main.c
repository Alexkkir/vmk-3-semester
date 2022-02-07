#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

enum { RADIX = 10, };

void handler(int signal);

void func(void);

int p[2];
int lim = -1, my_pid = -1, other_pid = -1, type = -1;

volatile sig_atomic_t working = 1, pid_1_written = 0, sig_flag;

FILE *in, *out;
int fd_in, fd_out;

void
handler(int s) {
    sig_flag = 1;
}

void handler_for_pid(int s) {
    pid_1_written = 1;
}

void
func(void) {
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
    }
    kill(other_pid, SIGUSR1);
}

int
main(int argc, char **argv) {
    lim = strtol(argv[1], NULL, RADIX);
    pipe(p);
    in = fdopen(p[0], "r");
    out = fdopen(p[1], "w");
    fd_in = p[0];
    fd_out = p[1];

    setbuf(in, NULL);
    setbuf(out, NULL);

    sigset_t s1, s2, s3;
    sigemptyset(&s1);
    sigaddset(&s1, SIGUSR1);
    sigprocmask(SIG_BLOCK, &s1, &s2);

    sigemptyset(&s1);
    sigaddset(&s1, SIGUSR2);
    sigprocmask(SIG_BLOCK, &s1, &s3);

    sigaction(SIGUSR1, &(struct sigaction){ .sa_handler = handler,
            .sa_flags = SA_RESTART }, NULL);
    sigaction(SIGUSR2, &(struct sigaction){ .sa_handler = handler_for_pid,
            .sa_flags = SA_RESTART }, NULL);

    if (!fork()) {
        signal(SIGUSR2, handler_for_pid);
        my_pid = getpid();
        type = 1;
        fprintf(out, "%d\n", my_pid);
        fflush(out);
        while(!pid_1_written) sigsuspend(&s3);
        fscanf(in, "%d", &other_pid);

        while (working) {
            sig_flag = 0;
            while(!sig_flag) sigsuspend(&s2);
            func();
        }

        _exit(1);
    }
    if (!fork()) {
        my_pid = getpid();
        type = 2;
        fscanf(in, "%d", &other_pid);
        kill(other_pid, SIGUSR2);
        fprintf(out, "%d\n", my_pid);
        fflush(out);

        fprintf(out, "0\n");
        fflush(out);

        kill(other_pid, SIGUSR1);
        while (working) {
            sig_flag = 0;
            while(!sig_flag) sigsuspend(&s2);
            func();
        }

        _exit(1);
    }
    close(p[0]);
    close(p[1]);
    wait(NULL);
    wait(NULL);
    printf("Done\n");
    return 0;
}