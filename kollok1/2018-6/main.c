#include <stdio.h>

unsigned func(unsigned idx, const unsigned *FAT) {
    int size = 1;
    while(FAT[idx] != 0) {
        idx = FAT[idx];
        size++;
    }
    return size;
}

int main() {
    unsigned FAT[] = {0, 2, 0, 0};
    printf("%d\n", func(1, FAT));
    return 0;
}
