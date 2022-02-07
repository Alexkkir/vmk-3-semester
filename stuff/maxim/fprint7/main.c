#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>

void
fprint7(FILE *f, int64_t number)
{
    if (number == 0) {
        fprintf(f, "0");
        return;
    }
    long long int temp[100] = {0};
    int pos = 0;
    if (number < 0) fprintf(f, "-");
    while (number != 0) {
        temp[pos++] = number % 7;
        if (temp[pos - 1] < 0) temp[pos - 1] *= -1;
        number /= 7;
    }
    for (int i = pos - 1; i >= 0; i--) fprintf(f, "%lld", temp[i]);
}

int
main()
{
    fprint7(stdout, -49);
}


