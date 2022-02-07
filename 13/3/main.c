#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

void print_arr(double ***arr) {
    for (int i = 0; arr[i]; i++) {
        for (int j = 0; arr[i][j]; j++) {
            printf("%.0f ", arr[i][j][0]);
        }
        printf("\n");
    }
}

double ***transpose(double ***arr) {
    int chunk = 20, reserved = 0, pos = 0;

    reserved = chunk;
    int *lens = malloc(reserved * sizeof(int));
    memset(lens, 0, reserved * sizeof(int));

    for (int i = 0; arr[i]; i++) {
        if (pos >= reserved) {
            reserved += chunk;
            lens = realloc(lens, reserved * sizeof(int));
            memset(lens + reserved - chunk, 0, chunk * sizeof(int));
        }
        for(int j = 0; arr[i][j]; j++)
            lens[i]++;
        pos++;
    }

    int max_wide = 0;
    for (int i = 0; i < pos; i++) {
        max_wide = MAX(lens[i], max_wide);
    }

    int *lens_2 = calloc(max_wide, sizeof(int));
    for (int j = 1; j <= max_wide; j++) {
        for (int i = pos - 1; i >= 0; i--) {
            if (lens[i] >= j) {
                lens_2[j - 1] = i + 1;
                break;
            }
        }
    }

    double ***arr_2 = calloc(max_wide + 1, sizeof(double **));
    for (int j = 0; j < max_wide; j++) {
        arr_2[j] = calloc(lens_2[j] + 1, sizeof(double *));
        for (int i = 0; i < lens_2[j]; i++) {
            if (j >= lens[i]) {
                arr_2[j][i] = calloc(1, sizeof(double ));
                arr_2[j][i][0] = 0;
            } else {
                arr_2[j][i] = calloc(1, sizeof(double ));
                arr_2[j][i][0] = arr[i][j][0];
            }
        }
    }

    free(lens);
    free(lens_2);

    return arr_2;
}


//int main() {
//    int lens[3] = {2, 3, 4};
//    int vals[3][10] = {{1}, {2, 3}, {4, 5, 6}};
//
//    double ***arr;
//    arr = calloc(4, sizeof(double **));
//    for (int i = 0; i < 3; i++) {
//        arr[i] = calloc(lens[i], sizeof(double *));
//        for (int j = 0; j < lens[i] - 1; j++) {
//            arr[i][j] = calloc(1, sizeof(double ));
//            *arr[i][j] = vals[i][j];
//        }
//    }
//
//    print_arr(arr);
//    double ***arr_2 = transpose(arr);
//    print_arr(arr_2);
//    return 0;
//}
