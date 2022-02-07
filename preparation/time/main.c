#include <stdio.h>
#include <time.h>

int main() {
    time_t today = time(NULL);
    struct tm *pt = localtime(&today);

    printf("Hello, World!\n");
    return 0;
}
