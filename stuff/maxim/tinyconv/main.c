#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>


void
tinyconv(uint8_t bytes[], size_t size)
{
    for (int i = 0; i < size; i++) {
        uint8_t res = 0;
        uint8_t temp = bytes[i];
        for (int j = 0; j < 8; j++) {
            res *= 2;
            res += temp % 2;

            temp /= 2;

        }
        bytes[i] = res;

    }
}

int
main()
{
    uint8_t b[] = {0xfa, 0x04};
    tinyconv(b, 2);
    for (int i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
        printf("%x ", b[i]);
    }
}

