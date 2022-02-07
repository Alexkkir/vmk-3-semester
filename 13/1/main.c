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
play_game(int my_idx) {
    while (semop(sem_id, (struct sembuf[]) {{my_idx, -1, 0}}, 1) >= 0) {
        int x = memory[0]++, prev_proc = memory[1];
        printf("%d %d %d\n", my_idx, x, prev_proc);
        fflush(stdout);
        memory[1] = my_idx;

        if (memory[0] > max_val) {
            shmctl(sh_mem_id, IPC_RMID, NULL);
            semctl(sem_id, 0, IPC_RMID);
            break;
        }

//        int next_idx = (((x * x) % n_processes) * ((x * x) % n_processes)) % n_processes + 1;
        int next_idx = (x * x * x * x) % n_processes + 1;
        semop(sem_id, (struct sembuf[]) {{next_idx, +1, 0}}, 1);
    }
    _exit(0);
}

int
main(int argc, char *argv[]) {
    n_processes = strtol(argv[1], NULL, 10);
    key = strtol(argv[2], NULL, 10);
    max_val = strtol(argv[3], NULL, 10);

    sem_id = semget(key, n_processes, 0777 | IPC_CREAT);

    sh_mem_id = shmget(key, 2 * sizeof(*memory), 0777 | IPC_CREAT);
    memory = shmat(sh_mem_id, NULL, 0);
    memory[0] = 0;
    memory[1] = 0;

    int *pids = calloc(n_processes, sizeof(pids[0]));
    for (int i = 0; i < n_processes; i++) {
        if (fork() == 0) {
            play_game(i);
        }
    }

    int first_process = 1;
    semop(sem_id, (struct sembuf[]) {{first_process, +1, 0}}, 1);

    for (int i = 0; i < n_processes; i++) wait(NULL);
    return 0;
}

