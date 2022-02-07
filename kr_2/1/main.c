#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

enum { CHUNK = 100, };

unsigned *arr = NULL, pos = 0, reserved = 0;

int
main()
{
    reserved += CHUNK;
    arr = malloc(reserved * sizeof(unsigned));

    unsigned x, max = 0;
    while (scanf("%u", &x) == 1) {
        if (pos >= reserved) {
            reserved += CHUNK;
            arr = realloc(arr, reserved * sizeof(unsigned));
        }
        arr[pos++] = x;
        max = MAX(x, max);
    }

    for (int i = pos - 1; i >= 0; i--) {
        if (arr[i] != 0 && max % arr[i] == 0) {
            printf("%u\n", arr[i]);
        }
    }

    free(arr);
    return 0;
}
