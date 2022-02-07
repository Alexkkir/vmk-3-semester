#include <stdio.h>
#include <stdlib.h>

char** cross(char **A, int n, int m) {
    int i = 0, j = 0;

    for (j = 0; j < n; j++)
        A[n][j] = '#';

    for (i = 0; j < m; i++)
        A[i][m] = '#';

    return A;
}


void arrout(char **A, int n, int m) {
    int i = 0, j = 0;

    for (i = 0; i < n; i++) {
        for (j = 0; j < m; j++) {
            printf("%c", A[i][j]);
        }
        printf("\n");
    }
}


int main() {
    int n = 0, m = 0;
    scanf("%d%d", &n, &m);
    printf("n=%d\nm=%d\n\n", n, m);

    char x;
    scanf("%c", &x);
    scanf("%c", &x);
    printf("%c", x);

    char **A = (char **) calloc(n, sizeof(char *));
    for (int i = 0; i < n; i++) {
        A[i] = (char *) calloc(m, sizeof(char));
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            scanf("%c", &A[i][j]);
            //    printf("%c",&A[i][j]);
        }
        //    printf("\n");
    }

    printf("\n\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%c", &A[i][j]);
        }
        printf("\n");
    }

    //    arrout(A, n, m);

cross(A, n, m);


        arrout(A, n, m);

    return 0;
}