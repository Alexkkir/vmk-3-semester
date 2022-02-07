#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
//#include <sys/wait.h>

int p[0];

int main() {
    pipe(p);
    FILE *f0, *f1;
    f0 = fdopen(p[0], "r");
    f1 = fdopen(p[1], "w");

    int x = 2;
    printf("here");
    fprintf(f1, "%d", x * 2);
    fscanf(f0, "%d", &x);
    printf("%d", x);
    return 0;
}
