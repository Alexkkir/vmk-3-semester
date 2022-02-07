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

int n_processes, key, iter_count, len;
int sem_id, mem_id, *memory;

void
operation(int *data, int ind1, int ind2, int value)
{
    if (ind1 != ind2) {
        int tmp1 = data[ind1] - value;
        int tmp2 = data[ind2] + value;

        data[ind1] = tmp1;
        data[ind2] = tmp2;
    }
}

int
rand_range(int low, int high)
{
    int num = 0;
    double random = 0;
    random = rand() / ((double) RAND_MAX + 1);
    num = low + (int) (random * (high - low));
    return num;
}

void
fun(int seed)
{
    srand(seed);

    for (int iter = 0; iter < iter_count; iter++) {
        int ind_1 = rand_range(0, len), ind_2 = rand_range(0, len), val = rand_range(0, 10); // generate random data
        if (ind_1 != ind_2) {
            semop(sem_id, (struct sembuf[]) {{ind_1, -1, 0},
                                             {ind_2, -1, 0}}, 2); // wait while free
            operation(memory, ind_1, ind_2, val); // perform operation
            semop(sem_id, (struct sembuf[]) {{ind_1, +1, 0},
                                             {ind_2, +1, 0}}, 2); // open semaphore
        }
    }
}

int
main(int argc, char **argv)
{
    sscanf(argv[1], "%d", &len);
    sscanf(argv[2], "%d", &key);
    sscanf(argv[3], "%d", &n_processes);
    sscanf(argv[4], "%d", &iter_count);

    int *random_seeds = calloc(n_processes, sizeof(int));
    for (int i = 0; i < n_processes; i++) {
        sscanf(argv[5 + i], "%d", &random_seeds[i]);
    }

    sem_id = semget(key, len, 0600 | IPC_CREAT);
    mem_id = shmget(key, len * sizeof(*memory), 0600 | IPC_CREAT);
    memory = shmat(mem_id, NULL, 0);

    for (int i = 0; i < len; i++) {
        scanf("%d", &memory[i]);
    }

    for (int i = 0; i < n_processes; i++) {
        if (!fork()) {
            fun(random_seeds[i]);
            _exit(0);
        }
    }

    // open all semaphores
    unsigned short sem_init_vals[len];
    for (int i = 0; i < len; i++) sem_init_vals[i] = 1;
    semctl(sem_id, 0, SETALL, sem_init_vals);

    // end
    while (wait(NULL) >= 0);
    for (int i = 0; i < len; i++) {
        printf("%d\n", memory[i]);
    }
    free(random_seeds);
    semctl(sem_id, 0, IPC_RMID);
    shmctl(mem_id, IPC_RMID, NULL);
    return 0;
}

