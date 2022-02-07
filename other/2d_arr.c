#include <stdio.h>

void fun_not_changing_array(int **a, int n, int m) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            sum += a[i][j];
        }
    }
    printf("%d", sum);
}

int main() {
    int n = 2, m = 3;
    int a[][] = {{1, 2}, {3, 4}, {5, 6}};
    fun(a, n, m);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            printf("%d ", a[i][j]);
        }
        printf("\n");
    }
}
