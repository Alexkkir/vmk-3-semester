#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <time.h>

struct info
{
    long infotype;
    char text[100];
};


int main ()
{
    key_t key1 = ftok("f", 'k');
    int queue = msgget(key1, IPC_CREAT | 0666);
    if (queue == -1) 
        perror("Open Error"); 
    
    struct info mesage1;
    puts("Enter number you want to guess");
    mesage1.infotype = 1;
    scanf("%s", mesage1.text);
    
    msgsnd(queue, &mesage1, sizeof(struct info), 0);    // send our number
    msgrcv(queue, &mesage1, sizeof(struct info), 2, 0); // function if good
    
    while (1)
    {
        mesage1.infotype = 1;
        puts("Try quess the number");
        scanf("%s", mesage1.text);

        msgsnd(queue, &mesage1, sizeof(struct info), 0);
        msgrcv(queue, &mesage1, sizeof(struct info), 2, 0);

        int tmp;
        sscanf(mesage1.text, "%d", &tmp);
        if (tmp)
            break;
    }
    
    return 0;

}
