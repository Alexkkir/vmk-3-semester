#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>
#include <signal.h>


enum { RADIX = 10, MAX_NUMBER_OF_PIPES = 2, NUM_FD_IN_PIPE = 2, BEGINNING_OF_MIDDLE_PART = 2 };

int pipes_arr[MAX_NUMBER_OF_PIPES][NUM_FD_IN_PIPE], pos_pipes = 0, *pids_arr, pos_pids = 0;
int pid, status;


void update_pos_pipes(void) {
    pos_pipes = (pos_pipes + 1) % MAX_NUMBER_OF_PIPES;
}

int prev_pipe(void) {
    return ((pos_pipes - 1) % MAX_NUMBER_OF_PIPES + MAX_NUMBER_OF_PIPES) % MAX_NUMBER_OF_PIPES;
}

void update_pids_arr() {
    pids_arr[pos_pids] = pid;
    pos_pids++;
}

void close_all_pipes() {
    close(pipes_arr[0][0]);
    close(pipes_arr[0][1]);
    close(pipes_arr[1][0]);
    close(pipes_arr[1][1]);
}

void terminate_all() {
    for (int i = 0; i < pos_pids; i++) {
        kill(pids_arr[i], SIGKILL);
        waitpid(pids_arr[i], NULL, 0);
    }
}


int main(int argc, char **argv) {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    int N = argc - 1;
    pids_arr = calloc(N, sizeof(pid_t));
    if (pids_arr == NULL) _exit(1);

    // check argv
    if (argc == 1) {
        return 0;
    }
    if (argc == 2) {
        if ((pid = fork()) == 0) {
            free(pids_arr);
            execlp(argv[1], argv[1], NULL);
            _exit(1);
        }
        if (pid == -1) {
            return 0;
        }
        waitpid(pid, NULL, 0);
        return 0;
    }

    // normal case: >= 2 commands

    // starting first one
    status = pipe(pipes_arr[pos_pipes]);
    if (status == -1) {
        exit(1);
    }
    if ((pid = fork()) == 0) {
        free(pids_arr);
        dup2(pipes_arr[pos_pipes][1], 1);
        close(pipes_arr[pos_pipes][0]);
        close(pipes_arr[pos_pipes][1]);
        execlp(argv[1], argv[1], NULL);
        _exit(1);
    } else {
        if (pid == -1) {
//            close_all_pipes();
            terminate_all();
            _exit(1);
        }
        update_pids_arr();
        close(pipes_arr[pos_pipes][1]);
        update_pos_pipes();
    }

    // starting all middle processes
    for (int i = BEGINNING_OF_MIDDLE_PART; i < argc - 1; i++) {
        status = pipe(pipes_arr[pos_pipes]);
        if (status == -1) {
            terminate_all();
            exit(1);
        }
        if ((pid = fork()) == 0) {
            free(pids_arr);
            dup2(pipes_arr[prev_pipe()][0], 0);
            dup2(pipes_arr[pos_pipes][1], 1);
            close_all_pipes();
            execlp(argv[i], argv[i], NULL);
            _exit(1);
        }
        if (pid == -1) {
//            close_all_pipes();
            terminate_all();
            _exit(1);
        }
        update_pids_arr();
        close(pipes_arr[pos_pipes][1]);
        close(pipes_arr[prev_pipe()][0]);
        update_pos_pipes();
    }

    // last process
    if ((pid = fork()) == 0) {
        free(pids_arr);
        dup2(pipes_arr[prev_pipe()][0], 0);
        close(pipes_arr[prev_pipe()][0]);
        close(pipes_arr[prev_pipe()][1]);
        execlp(argv[N], argv[N], NULL);
        _exit(1);
    }
    if (pid == -1) {
//        close_all_pipes();
        terminate_all();
        _exit(1);
    }
    update_pids_arr();
    close(pipes_arr[prev_pipe()][0]);

    for (int i = 0; i < pos_pids; i++) {
        status = waitpid(pids_arr[i], NULL, 0);
            if (status == -1) {
                terminate_all();
                _exit(1);
            }
    }
    return 0;
}