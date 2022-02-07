#include <stdio.h>
#include <sys/types.h>

#define SIZE 1024
double a[SIZE][SIZE];
int i;
double b[SIZE][SIZE];
void main(int argc, char **argv) {
    unsigned cycles_high = 0, cycles_low = 0;
    unsigned cycles_high1 = 0, cycles_low1 = 0;
    asm volatile ("CPUID\n\t"
                  "RDTSC\n\t"
                  "mov %%edx, %0\n\t"
                  "mov %%eax, %1\n\t": "=r" (cycles_high), "=r" (cycles_low)::
    "%rax", "%rbx", "%rcx", "%rdx");
/***********************************/
    int j, k;
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            b[i][j] = 20.19;
        }
    }
    for (i = 0; i < SIZE; i++) {
        for (j = 0; j < SIZE; j++) {
            a[i][j] = 0;
            for (k = 0; k < SIZE; k++)
                a[i][j] += b[i][k] + b[j][k];
        }
    }
/***********************************/
    asm volatile("RDTSCP\n\t"
                 "mov %%edx, %0\n\t"
                 "mov %%eax, %1\n\t"
                 "CPUID\n\t": "=r" (cycles_high1), "=r" (cycles_low1)::
    "%rax", "%rbx", "%rcx", "%rdx");

    unsigned long long start = ( ((unsigned long long)cycles_high << 32) | cycles_low );
    unsigned long long end = ( ((unsigned long long)cycles_high1 << 32) | cycles_low1 );
    printf("%lld, %lld, %lld", start, end, end - start);
}

// 50, 13, 8, 8, 10, 12 / 0.001
// 7063494735 5873514161 6190740965
//160000000