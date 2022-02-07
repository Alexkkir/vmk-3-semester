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
    long type;
    char infotext[100];
};

struct sysinfo
{
    long type;
    char infosygnal[100];
};

void function(int queue, struct info* number)
{
	int str[6];
	for(int i=0; i<6; i++)
	{
		str[i]=0;
	}
	
	int ox=0;
	int cows=0;
	
    number->type = 1;
    struct sysinfo systeminfo;
    systeminfo.type = 2;
    sprintf(systeminfo.infosygnal, "%d", 0);
    int n;
    sscanf(number->infotext, "%d", &n);
    printf("number is: %d\n", n);
    msgsnd(queue, &systeminfo, sizeof(struct sysinfo), 0);
    msgrcv(queue, number, sizeof(struct info), 1, 0);
    while (1)
    {
    	/*
        if((number -> infotext) > n)
        {
            puts("<");
            msgsnd(queue, &systeminfo, sizeof(struct sysinfo), 0);  
            msgrcv(queue, number, sizeof(struct info), 1, 0);  
        }
        else
        {
            if ((number -> infotext) < n)
            {
            	puts(">");
            	msgsnd(queue, &systeminfo, sizeof(struct sysinfo), 0);
            	msgrcv(queue, number, sizeof(struct info), 1, 0);
            }
            else
            {
                puts("Correct!");
                systeminfo.infosygnal = 1;
                msgsnd(queue, &systeminfo, sizeof(struct sysinfo), 0);
                break;
            }
        }
        */   
        int a;
        sscanf(number -> infotext, "%d", &a); //from queue
        int b=n; //number
        printf("a: %d, b: %d \n",a,b);
             
        for(int i=3; i>0; i--)
        {
        	if(a%(10*i)==(b%(10*i)))
        	{
        		ox++;
        		//printf("%d", a%(10*i));
        	}
        		
        		
        	if(a%(10*i)!=(b%(10*i)))
        	{
        		str[a%(10*i)]++;
        		str[b%(10*i)]++;
        		
        		//printf("%d", a%(10*i));
        	}
        }
        
        for(int i=0; i<6; i++)
        {
        	if(str[i]==2)
        		cows++;
        		
        	str[i]=0;
        }
        
        if(ox==4)
        {
        	ox=0;
        	cows=0;
        	
        	puts("Correct!");
        	sprintf(systeminfo.infosygnal, "%d", 1);
            msgsnd(queue, &systeminfo, sizeof(struct sysinfo), 0);
            break;
        }
        else
        {
        	//printf("ox: %d, cows: %d\n",ox,cows);
        	
        	a=a/10;
        	b=b/10;
        	ox=0;
        	cows=0;
        	
        	msgsnd(queue, &systeminfo, sizeof(struct sysinfo), 0);  
        	msgrcv(queue, number, sizeof(struct info), 1, 0); 
        }
    }
}

int main()
{
    key_t key = ftok("f", 'k');
    int queue = msgget(key, IPC_CREAT | 0666);
    if (queue == -1) 
        perror("Open Error"); 

    struct info number;
    number.type = 1;

    struct sysinfo syncing;
    syncing.type = 2;
    
    msgrcv(queue, &number, sizeof(number), 1, 0);

    function(queue, &number);

    msgctl(queue, IPC_RMID, 0);  // delete 
    return 0;
}
