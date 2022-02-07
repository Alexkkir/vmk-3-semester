#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

enum {
    RADIX = 10,
    MAX_NUMBER_OF_PIPES = 2,
    NUM_FD_IN_PIPE = 2
};

int pipes_arr[MAX_NUMBER_OF_PIPES][NUM_FD_IN_PIPE], pos_pipes = 0, *pids_arr, pos_pids = 0;

void
terminate() {
    for (int i = 0; i < pos_pids; i++) {
        kill(pids_arr[i], SIGKILL);
    }
}

void update_pos_pipes(void) {
    pos_pipes = (pos_pipes + 1) % MAX_NUMBER_OF_PIPES;
}

void get_prev_pos_pipes(void) {
    pos_pipes = ((pos_pipes - 1) % MAX_NUMBER_OF_PIPES + MAX_NUMBER_OF_PIPES) % MAX_NUMBER_OF_PIPES;
}


int
main(int argc, char **argv) {
    int n = argc - 1;
    pids_arr = calloc(n, sizeof(int *));
    int pid, status;

    // check number of commands
    if (argc == 1) {
        return 0;
    } else if (argc == 2) {
        if (!fork()) {
            execlp(argv[1], argv[1], NULL);
            _exit(2);
        }
        wait(NULL);
        return 0;
    }

    // first command
    status = pipe(pipes_arr[pos_pipes]);
    if (status == -1) _exit(1);

    if ((pid = fork()) == 0) {
        free(pids_arr);
        dup2(pipes_arr[pos_pipes][1], 1);
        close(pipes_arr[pos_pipes][0]);
        close(pipes_arr[pos_pipes][1]);
        execlp(argv[1], argv[1], NULL);
        _exit(2);
    }

    wait(&status); // here would be error, if process wasn't created
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("error in  1\n");
        terminate();
        _exit(3);
    }

    pids_arr[pos_pids] = pid;
    pos_pids++;

    close(pipes_arr[pos_pipes][1]);
    update_pos_pipes();

    // middle commands
    for (int i = 2; i < argc - 1; i++) {
        status = pipe(pipes_arr[pos_pipes]);
        if (status == -1) _exit(1);
        if ((pid = fork()) != 0) {
            free(pids_arr);
            dup2(pipes_arr[pos_pipes - 1][0], 0);
            dup2(pipes_arr[pos_pipes][1], 1);
            close(pipes_arr[pos_pipes - 1][0]);
            close(pipes_arr[pos_pipes][0]);
            close(pipes_arr[pos_pipes][1]);
            execlp(argv[i], argv[i], NULL);
            _exit(2);
        }

        wait(&status); // if can't creat process
        if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
            printf("error in  2\n");
            terminate();
            _exit(3);
        }

        pids_arr[pos_pids] = pid;
        pos_pids++;
        close(pipes_arr[pos_pipes - 1][0]);
        close(pipes_arr[pos_pipes][1]);
        update_pos_pipes();
    }

    // last command
    get_prev_pos_pipes();
    if ((pid = fork()) != 0) {
        free(pids_arr);
        dup2(pipes_arr[pos_pipes][0], 0);
        close(pipes_arr[pos_pipes][0]);
        execlp(argv[argc - 1], argv[argc - 1], NULL);

        pids_arr[pos_pids] = pid; // bad case
        pos_pids++;
        terminate();
        _exit(2);
    }

    wait(&status); // if can't creat process
    if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
        printf("error in  3\n");
        terminate();
        _exit(3);
    }

    pids_arr[pos_pids] = pid;
    pos_pids++;

    return 0;
}
