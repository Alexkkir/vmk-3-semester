#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

int n_processes, key, max_val, sem_id, *memory, sh_mem_id;

void
play_game(int my_idx)
{
    while (semop(sem_id, (struct sembuf[]) {{my_idx - 1, -1, 0}}, 1) >= 0) {
        int val = memory[0] + 1;
        val %= n_processes;
        val *= val;
        val %= n_processes;
        val *= val;
        val %= n_processes;
        printf("%d %d %d\n", my_idx, memory[0]++, memory[1]);
        memory[1] = my_idx;
        fflush(stdout);
        if (memory[0] > max_val) {
            semctl(sem_id, 0, IPC_RMID);
            shmctl(sh_mem_id, IPC_RMID, NULL);
            break;
        }
        semop(sem_id, (struct sembuf[]) {{val, 1, 0}}, 1);
    }
    _exit(0);
}

int
main(int argc, char *argv[])
{
    n_processes = strtol(argv[1], NULL, 10);
    key = strtol(argv[2], NULL, 10);
    max_val = strtol(argv[3], NULL, 10);

    sem_id = semget(key, n_processes, 0600 | IPC_CREAT);

    sh_mem_id = shmget(key, 2 * sizeof(*memory), 0600 | IPC_CREAT);
    memory = NULL;
    memory = shmat(sh_mem_id, NULL, 0);
    memory[0] = 0;
    memory[1] = 0;

    for (int i = 1; i <= n_processes; ++i) {
        if (fork() == 0) {
            play_game(i);
            exit(0);
        }
    }

    int first_process = 0;
    semop(sem_id, (struct sembuf[]) {{first_process, +1, 0}}, 1);

    while (wait(NULL) >= 0);
    return 0;
}

