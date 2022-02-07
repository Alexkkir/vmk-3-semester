#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum { SIZE = 500, KEY = 11223344};

int msg_id, n;

char str[SIZE];

struct message {
    long type;
    char str[SIZE];
};

void fun(int my_id) {
    struct message m;
    while (msgrcv(msg_id, &m, SIZE, my_idx + 1, 0) != -1) {

    }
}

int main() {
    msg_id = msgget(KEY, 0777 | IPC_CREAT);
    scanf("%d", &n);

    for (int i = 0; i < 3; i++) {
        if (!fork()) {
            fun(i + 1);
            _exit(0);
        }
    }
    struct message;
}