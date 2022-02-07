#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>

volatile sig_atomic_t confirm_receipt, end_of_file = 0;
int other_pid, my_pid;

void
WriteBit(unsigned char bit);

void
CheckAndPrint();

void
Handler(int s)
{
    switch (s) {
        case SIGUSR1:
            WriteBit(0);
            CheckAndPrint();
            kill(other_pid, SIGALRM);
            break;
        case SIGUSR2:
            WriteBit(1);
            CheckAndPrint();
            kill(other_pid, SIGALRM);
            break;
        case SIGALRM:
            confirm_receipt = 1;
            break;
        case SIGIO:
            end_of_file = 1;
            break;
        default:
            break;
    }
}

unsigned char buff = 0;
int buff_pos = 0;

void
WriteBit(unsigned char bit)
{
    buff |= bit << buff_pos;
    buff_pos++;
}

void
CheckAndPrint()
{
    if (buff_pos == 8) {
        printf("%c", buff);
        fflush(stdout);
        buff_pos = 0;
        buff = 0;
    }
}

int
main(int argc, char **argv)
{
    int p1[2], p2[2];
    pipe(p1), pipe(p2);

    sigset_t s1, s2;
    sigemptyset(&s1);
    sigaddset(&s1, SIGUSR1);
    sigaddset(&s1, SIGUSR2);
    sigaddset(&s1, SIGALRM);
    sigaddset(&s1, SIGIO);
    sigprocmask(SIG_BLOCK, &s1, &s2);

    sigaction(SIGUSR1, &(struct sigaction) {.sa_handler = Handler, .sa_flags = SA_RESTART}, NULL);
    sigaction(SIGUSR2, &(struct sigaction) {.sa_handler = Handler, .sa_flags = SA_RESTART}, NULL);
    sigaction(SIGALRM, &(struct sigaction) {.sa_handler = Handler, .sa_flags = SA_RESTART}, NULL);
    sigaction(SIGIO, &(struct sigaction) {.sa_handler = Handler, .sa_flags = SA_RESTART}, NULL);

    if (fork() == 0) {
        // receiver
        my_pid = getpid();
        write(p1[1], &my_pid, sizeof(my_pid));
        read(p2[0], &other_pid, sizeof(other_pid));
        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);

        while (!end_of_file) sigsuspend(&s2);
        _exit(0);
    }
    if (fork() == 0) {
        // sender
        my_pid = getpid();
        write(p2[1], &my_pid, sizeof(my_pid));
        read(p1[0], &other_pid, sizeof(other_pid));
        close(p1[0]);
        close(p1[1]);
        close(p2[0]);
        close(p2[1]);

        int fd = open(argv[1], O_RDONLY);
        int size = lseek(fd, 0, SEEK_END);
        lseek(fd, 0, SEEK_SET);
        for (int n_byte = 0; n_byte < size; n_byte++) {
            unsigned char byte;
            read(fd, &byte, sizeof(unsigned char));
            for (int i = 0; i < 8; i++) {
                int bit = (byte >> i) & 1;
                switch (bit) {
                    case 0:
                        kill(other_pid, SIGUSR1);
                        break;
                    case 1:
                        kill(other_pid, SIGUSR2);
                        break;
                    default:
                        break;
                }
                confirm_receipt = 0;
                while (!confirm_receipt) sigsuspend(&s2);
            }
        }
        kill(other_pid, SIGIO);
        close(fd);
        _exit(0);
    }

    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);

    wait(NULL);
    wait(NULL);
    return 0;
}
