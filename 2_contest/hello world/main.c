#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    printf("Command: -> ");
    for(int i = 0; i < argc; i++) {
        printf(i != argc - 1 ? "%s|" : "%s <-", argv[i]);
    }
        return 0;
}