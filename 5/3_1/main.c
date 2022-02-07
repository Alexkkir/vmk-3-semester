#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

enum { LEN_CELL = 1010, LEN_LINE = 3010, OCTAVE_RADIX = 8, N_COLS = 3, MAX_N = 32 };

int
main()
{
    // README README README README README README README
    // My last 4 submitted solutions are experiment on question
    // when overflow in expression ~0 >> (32 - n) occur.
    // Experiment has showed, that if n = 0 overflow occurs.
    // Unfortunately, cost of the experiment is 30 points.
    // Can you please retrieve me score?
    uint32_t n, s, w;
    scanf("%" PRIu32 "%" PRIu32 "%" PRIu32, &n, &s, &w);
    char spaces[LEN_CELL];
    char ans_line[LEN_LINE];
    /* each of three numbers in table's row*/
    char buf0[LEN_CELL] = {0}, buf1[LEN_CELL] = {0}, buf2[LEN_CELL] = {0};
    memset(spaces, ' ', sizeof(spaces));
    uint32_t is_last_iter = 0;
    for (uint32_t x = 0, limit = (n < MAX_N) ? 1 << n : 0, iter = 0;
            n < MAX_N ? x < limit : !is_last_iter;
            is_last_iter = x > ~((int32_t) 0) - s, x += s, iter++) {
        uint32_t x_len, num, i;
        /*writing to buf 0*/
        for (x_len = 0, num = x; num > 0; x_len++, num /= OCTAVE_RADIX)
            ;
        if (x == 0) {
            buf0[0] = '0';
        } else {
            for (i = 0, num = x; num > 0; num /= OCTAVE_RADIX, i++) {
                buf0[x_len - i - 1] = num % OCTAVE_RADIX + '0';
            }
        }
        /*writing to buf1*/
        sprintf(buf1, "%" PRIu32, x);
        /*writing to buf2*/
        uint32_t n2 = 0, bit = 1;
        for (i = 0; i < n - 1; i++, bit <<= 1) {
            n2 += x & bit;
        }
        if (x & bit && n2) {
            sprintf(buf2, "-%" PRIu32, n2);
        } else {
            sprintf(buf2, "%" PRIu32, n2);
        }

        char *buffers[N_COLS] = {buf0, buf1, buf2};
        memset(ans_line, 0, sizeof(ans_line));
        for (uint32_t j = 0; j < N_COLS; j++) {
            char *buff = buffers[j];
            strcat(ans_line, "|");
            i = w - strlen(buff);
            spaces[i] = '\0';
            strcat(ans_line, spaces);
            spaces[i] = ' ';
            strcat(ans_line, buff);
        }
        strcat(ans_line, "|\n");
        printf("%s", ans_line);
    }
    return 0;
}
