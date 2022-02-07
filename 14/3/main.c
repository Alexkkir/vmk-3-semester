#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdint.h>
#include <inttypes.h>

int n_processes, key, msg_id;
int64_t val_1, val_2, max_val;

enum {
    SIZE = 200,
};

#define ABS(x) ((x) > 0 ? (x) : -(x))

struct my_buf {
    long msgtype;
    char msgtext[SIZE];
};

void
fun(int my_idx)
{
    struct my_buf s;
    // msgrcv = -1  <=>  queue of messages is deleted
    while (msgrcv(msg_id, &s, SIZE, my_idx + 1, 0) != -1) {
        sscanf(s.msgtext, "%" SCNd64 "%" SCNd64, &val_1, &val_2);
        int64_t val_3 = val_1 + val_2;

        printf("%d %" PRId64 "\n", my_idx, val_3);
        fflush(stdout);
        if (ABS(val_3) > ABS(max_val)) {
            msgctl(msg_id, IPC_RMID, NULL);
        }

        s.msgtype = val_3 % n_processes + 1;
        sprintf(s.msgtext, "%" PRId64 " %" PRId64, val_2, val_3);
        msgsnd(msg_id, &s, SIZE, 0);
    }
}

int
main(int argc, char **argv)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    // data input
    key = strtol(argv[1], NULL, 10);
    n_processes = strtol(argv[2], NULL, 10);
    sscanf(argv[3], "%" SCNd64, &val_1);
    sscanf(argv[4], "%" SCNd64, &val_2);
    sscanf(argv[5], "%" SCNd64, &max_val);

    msg_id = msgget(key, 0600 | IPC_CREAT);

    // start processes
    int *pids = calloc(n_processes, sizeof(int)), pid, pos = 0;
    for (int i = 0; i < n_processes; i++) {
        if ((pid = fork()) == 0) {
            // run function
            fun(i);
            _exit(0);
        } else {
            if (pid != -1) {
                // add current pid to set of pids
                pids[pos++] = pid;
            } else {
                // stop all
                for (int j = 0; j < pos; j++) {
                    kill(pids[j], SIGKILL);
                    wait(NULL);
                }
                free(pids);
                msgctl(msg_id, IPC_RMID, NULL);
                _exit(1);
            }
        }
    }

    // prepare struct
    int first = 0;
    struct my_buf s = {.msgtype = first + 1};
    sprintf(s.msgtext, "%" PRId64 " %" PRId64, val_1, val_2);

    // send message and wait until end
    msgsnd(msg_id, &s, SIZE, 0);
    while (wait(NULL) >= 0);

    free(pids);
    return 0;
}

