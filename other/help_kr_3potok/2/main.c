#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/wait.h>

int sem_id, sh_mem_id, *memory;
int father_id = 2;

enum { MY_EOF = 10000001 };


void fun(int my_id) {
    int sum = 0;
    while (semop(sem_id, (struct sembuf[]) {{my_id, -1, 0}}, 1) >= 0) {
        if (*memory == MY_EOF) {
            if (my_id == 0) {
                printf("son %d\n", sum);
            } else {
                printf("son2 %d\n", sum);
            }
            fflush(stdout);
        }
        sum += *memory;
//        printf("> %d %d\n", my_id, *memory);

        semop(sem_id, (struct sembuf[]) {{father_id, +1, 0}}, 1) >= 0;
    }

}

int
main()
{
    sem_id = semget(11223344, 3, 0777 | IPC_CREAT);

    int mem_size = 1;
    sh_mem_id = shmget(11223344, mem_size * sizeof(*memory), 0777 | IPC_CREAT);
    memory = NULL;
    memory = shmat(sh_mem_id, NULL, 0);
    *memory = 0;

    for (int i = 0; i < 2; i++) {
        if (!fork()) {
            fun(i);
            exit(0);
        }
    }

    int x;
    int n_son;
    while(scanf("%d", &x) > 0) {
        *memory = x;
        semop(sem_id, (struct sembuf[]) {{n_son, +1, 0}}, 1);
        n_son = (n_son + 1) % 2;
        semop(sem_id, (struct sembuf[]) {{father_id, -1, 0}}, 1);
    }

    for (n_son = 0; n_son < 2; n_son++) {
        *memory = MY_EOF;
        semop(sem_id, (struct sembuf[]) {{n_son, +1, 0}}, 1);
        semop(sem_id, (struct sembuf[]) {{father_id, -1, 0}}, 1);
    }

    shmctl(sh_mem_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    while (wait(NULL) >= 0);

    return 0;
}
