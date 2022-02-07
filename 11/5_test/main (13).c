Хребтов Максим [205 группа, 3 семестр, 2021-2022]: Source for run 4659
Settings
Registration

Info
Summary
Submissions
Submit clar
Clars

Logout [s02200510]
logo
00:50:28 / 1 unread message(s) / RUNNING
Source code
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <sys/wait.h>
#include <time.h>
int main(int argc,char **argv)
{

    int pfd1[2];
    int pfd2[2];
    pipe(pfd1);
    pipe(pfd2);
    FILE *in1=fdopen(pfd1[0],"r");
    FILE *in2=fdopen(pfd2[0],"r");
    FILE *out1=fdopen(pfd1[1],"w");
    FILE *out2=fdopen(pfd2[1],"w");
    int x=1;
    int limit=atoi(argv[1]);

    if(!fork()) {
        fclose(in2);
        fclose(out1);
        setbuf(in1,NULL);
        while(1) {

            int x=0;
            int s=fscanf(in1,"%d",&x);
            if(s<=0) {
                break;
            }
            if(x==limit) {
                break;
            }
            printf("1 %d\n",x);
            fflush(stdout);
            x+=1;
            fprintf(out2,"%d ",x);
            fflush(out2);
        }
        fclose(in1);
        fclose(out2);
        exit(0);
    }
    if(!fork()) {
        fclose(in1);
        fclose(out2);
        setbuf(in2,NULL);
        while(1) {
            int x=0;
            int s=fscanf(in2,"%d",&x);
            if(s<=0) {
                break;
            }
            if(x==limit) {
                break;
            }
            printf("2 %d\n",x);
            fflush(stdout);
            x+=1;

            fprintf(out1,"%d ",x);
            fflush(out1);
        }
        fclose(in2);
        fclose(out1);
        exit(0);
    }
    fprintf(out1,"%d ",x);
    fflush(out1);
    fclose(in1);
    fclose(out1);
    fclose(in2);
    fclose(out2);
    wait(NULL);
    wait(NULL);


    printf("Done\n");
return 0;
}
Resubmit  Download run
Run comments
Author	Run comment
Judge
2021/11/28 15:40:34
дублирование кода
ejudge 3.8.0+ (GIT 48acc94) #3 (2021-11-14 00:01:39).

Copyright © 2000-2021 Alexander Chernov.
