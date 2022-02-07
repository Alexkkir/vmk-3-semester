#define cpuid __asm __emit 0fh __asm __emit 0a2h
#define rdtsc __asm __emit 0fh __asm __emit 031h
#include <stdio.h>
void main () {
    unsigned long long a[] = {7063494735, 5873514161, 6190740965}, g = 160000000, b[] = {6121777451, 5819762263, 5806604269};
    for (int i = 0; i < 3; i++) {
        printf("%lld ", b[i] / g);
    }
}