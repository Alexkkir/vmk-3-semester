#include <stdio.h>
#include <stdlib.h>

void fun_not_changing(int **a, int n, int m) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += a[i][j];
            a[i][j] *= 2;
        }
    }
    printf("\n%d\n", sum);
}


int main() {
    int n, m;
    scanf("%d%d", &n, &m);
    int **arr = (int**) calloc (n, sizeof(int*));
    for (int i = 0; i < n; i++) {
        arr[i] = (int*) calloc (m, sizeof(int));
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            scanf("%d", &arr[i][j]);
        }
    }
    arr[0][0] = 0; arr[0][1] = 1; arr[1][0] = 2; arr[1][1] = 3;
    // тут типо считываем массив
    fun_not_changing(arr, n, m);

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%d ", arr[i][j]);
        }
        printf("\n");
    }
}
