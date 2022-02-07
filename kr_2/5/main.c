#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

typedef unsigned long long ull;

ull n_processes, max_hp, sem_id, *memory, sh_mem_id;

volatile sig_atomic_t flag = 0;

void handl(int s) {
    signal(SIGUSR1, handl);
    flag = 1;
}

void
play_game(ull my_idx)
{
    while (semop(sem_id, (struct sembuf[]) {{my_idx, -1, 0}}, 1) >= 0) {
        ull my_hp = memory[my_idx];
        printf("%llu %llu\n", my_idx, my_hp);
        fflush(stdout);

        if (my_hp == 0) {
            kill(getppid(), SIGUSR1);
            _exit(0);
        }
        kill(getppid(), SIGUSR1);
    }
    _exit(0);
}

int
main(int argc, char **argv)
{
//    printf("asasasas\n");
    // init
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    ull key = 11223344;
    n_processes = strtol(argv[1], NULL, 10);
    max_hp = strtol(argv[2], NULL, 10);

    sem_id = semget(key, n_processes, 0777 | IPC_CREAT);

    memory = NULL;
    sh_mem_id = shmget(key, n_processes * sizeof(*memory), 0777 | IPC_CREAT);
    memory = shmat(sh_mem_id, NULL, 0);
    for (ull i = 0; i < n_processes; i++) {
        memory[i] = max_hp;
    }

    // init players
    for (ull i = 0; i < n_processes; i++) {
        if (!fork()) {
            play_game(i);
            exit(0);
        }
    }

    // playing
    ull player = 0;
    const ull damage = 1;
    signal(SIGUSR1, handl);
    while (scanf("%llu", &player) == 1) {
        player = ((player % n_processes) + n_processes) % n_processes;
        if (memory[player] > 0) {
            memory[player] -= damage;
            semop(sem_id, (struct sembuf[]) {{player, +1, 0}}, 1);
        }
        pause();
    }

    // stop play
    semctl(sem_id, 0, IPC_RMID);
    shmctl(sh_mem_id, IPC_RMID, NULL);

    // results of game
    while (wait(NULL) >= 0);
    ull sum = 0;
    for (ull i = 0; i < n_processes; i++) {
        if (memory[i] > 0) {
            sum++;
        }
    }
    printf("%llu\n", sum);
    return 0;
}

