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

enum { ACCESS_RIGHTS = 0600, RANDOM_KEY = 3301, };

int n_processes, sem_id;

void fun(int my_id) {
    while(semop(sem_id, (struct sembuf[]) {{my_id, -1, 0}}, 1) >= 0) {
        int x;
        if (scanf("%d", &x) != 1) {
            // end of stdin
            semctl(sem_id, 0, IPC_RMID);
            break;
        }
        printf("%d %d\n", my_id, x);
        int next_id = (x % n_processes + n_processes) % n_processes;
        semop(sem_id, (struct sembuf[]) {{next_id, +1, 0}}, 1);
    }
}

int
main(int argc, char **argv)
{
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    sscanf(argv[1], "%d", &n_processes);
    sem_id = semget(RANDOM_KEY, n_processes, ACCESS_RIGHTS | IPC_CREAT | IPC_PRIVATE);

    for (int i = 0; i < n_processes; i++) {
        if (!fork()) {
            fun(i);
            exit(0);
        }
    }

    int first = 0;
    semop(sem_id, (struct sembuf[]) {{first, +1, 0}}, 1);

//    usleep(2000000);
//    semctl(sem_id, 0, IPC _RMID);
    while (wait(NULL) >= 0);
    return 0;
}
