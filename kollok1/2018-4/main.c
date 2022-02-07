#include <stdio.h>
#include <signal.h>

volatile sig_atomic_t Counter1 = 0, Counter2 = 0, Counter3 = 0;

void handler(int s) {
    signal(Sig1, handler);
    signal(Sig2, handler);
    signal(Sig3, handler);
    if (s == Sig1) Counter1++;
    if (s == Sig2) Counter2++;
    if (s == Sig3) Counter3++;
}

int main() {
    signal(Sig1, handler);
    signal(Sig2, handler);
    signal(Sig3, handler);

    while(1) pause();
    return 0;
}
