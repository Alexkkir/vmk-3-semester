#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "ppm.h"

#define b(i) b[i + 1]
#define b_cont1(i) b_cont1[c_prev][i + 1]
#define b_cont2(i) b_cont2[context_2[0]][context_2[1]][i + 1]
#define b_cont2_check(i) (b_cont2[context_2[0]][context_2[1]] ? b_cont2[context_2[0]][context_2[1]][i + 1] : i + 1)
// умная (нет) реализация
//#define sum_b(i) (b_cont2(N_SYMBOLS - 1) > (200) ? b_cont2(i) : b_cont1(i))
//#define sum_b_check(i) (b_cont2_check(N_SYMBOLS - 1) > (200) ? b_cont2_check(i) : b_cont1(i))
// прямая реализация
#define sum_b(i) b_cont2(i)
#define sum_b_check(i) b_cont2_check(i)
#define max(a, b) a > b ? a : b
typedef uint64_t interval_t;

static const interval_t INTERVAL_LEN = (1ll << 32) - 1,
        FIRST_QTR = (INTERVAL_LEN + 1) / 4,
        HALF = 2 * FIRST_QTR,
        THIRD_QTR = 3 * FIRST_QTR;
enum {
    BUF_LEN = 8192,
    LEN_SYMBOL = 1,
    N_SYMBOLS = 256,
    ADD_TO_END = 100,
    BYTE_LEN = 8,
    BUF_LAST_IND = BUF_LEN,
    BUF_LAST_BIT = BUF_LEN * LEN_SYMBOL * BYTE_LEN,
};

static int THRESHOLD = 8192 * 16, AGGRESSIVENESS = 80;
static unsigned char buff_write[BUF_LEN] = {0}, buff_read[BUF_LEN] = {0};
static int buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
static int bits_to_follow = 0;
static FILE *ifp, *ofp;

static void write_bit(int bit);

static void bits_plus_follow(int bit);

static int get_bit();

static int get_symbol();

static void write_symbol(int symbol);

void compress_ppm(char *ifile, char *ofile) {
    ifp = (FILE *) fopen(ifile, "rb");
    ofp = (FILE *) fopen(ofile, "wb");

    memset(buff_read, 0, sizeof(buff_read));
    memset(buff_write, 0, sizeof(buff_write));
    buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
    bits_to_follow = 0;

    // definition of aux structures
    interval_t b[N_SYMBOLS + 1];
    for (int i = 0; i <= N_SYMBOLS; i++) {
        b[i] = i;
    }

    int a = sizeof(size_t);

    interval_t b_cont1[N_SYMBOLS][N_SYMBOLS + 1] = {0};
    for (int i = 0; i < N_SYMBOLS; i++) {
        for (int j = 0; j < N_SYMBOLS + 1; j++) {
            b_cont1[i][j] = j;
        }
    }

    interval_t *b_cont2[N_SYMBOLS][N_SYMBOLS] = {0};

    fseek(ifp, 0, SEEK_END);
    int n_symbols = ftell(ifp);
    fwrite(&n_symbols, sizeof(n_symbols), 1, ofp);
    fseek(ifp, 0, SEEK_SET);

    interval_t li = 0, hi = 0, li_prev = 0, hi_prev = INTERVAL_LEN, divider;
    int add_to_end = ADD_TO_END, max_freq_ind, found = 0;
    interval_t max_freq;

    int c, c_prev = 0, context_2[2] = {0}; // depends on constants
    int index;
    int n_iter;
    for (n_iter = 0;; n_iter++) {
//        printf("%d\n", n_iter);
        c = get_symbol(); // Читаем символ
        if (c == -1 || n_iter >= n_symbols) {
            if (add_to_end > 0) {
                if (found == 0) {
                    found = 1, max_freq = 0, max_freq_ind = 0;
                    for (int i = 0; i < N_SYMBOLS; i++) {
                        interval_t freq = sum_b_check(i) - sum_b_check(i - 1);
                        if (freq > max_freq) {
                            max_freq = freq;
                            max_freq_ind = i;
                        }
                    }
                    index = max_freq_ind;
                    c = max_freq_ind;
                }
                add_to_end--;
            } else
                break;
        } else {
            index = c;
        }
        if (!b_cont2[context_2[0]][context_2[1]]) {
            b_cont2[context_2[0]][context_2[1]] = calloc(N_SYMBOLS + 1, sizeof(interval_t));
            for (int i = 0; i < N_SYMBOLS; i++) {
                b_cont2(i) = i + 1;
            }
        }
        divider = sum_b(N_SYMBOLS - 1);
        li = li_prev + sum_b(index - 1) * (hi_prev - li_prev + 1) / divider;
        hi = li_prev + sum_b(index) * (hi_prev - li_prev + 1) / divider - 1;
        // обработка вариантов переполнения
        int xxx = 1;
        for (;;) {
            if (hi < HALF) {
                bits_plus_follow(0);
            } else if (li >= HALF) {
                bits_plus_follow(1);
                li -= HALF;
                hi -= HALF;
            } else if ((li >= FIRST_QTR) && (hi < THIRD_QTR)) {
                bits_to_follow++;
                li -= FIRST_QTR;
                hi -= FIRST_QTR;
            } else
                break;
            li <<= 1;
            hi <<= 1;
            hi += 1;
        }
        li_prev = li, hi_prev = hi;

        // обновление интервалов
        for (int i = index; i < N_SYMBOLS; i++) {
            b(i) += AGGRESSIVENESS;
            b_cont1(i) += AGGRESSIVENESS;
            b_cont2(i) += AGGRESSIVENESS;
        }
        // сброс таблицы
        interval_t total_delta, table_i, old_b_i_prev;
        total_delta = 0, table_i, old_b_i_prev = 0;
        if (b(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = b(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = b(i);
                b(i) -= total_delta;
            }
        }
        // сброс контекстуальной таблицы
        total_delta = 0, table_i, old_b_i_prev = 0;
        if (b_cont1(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = b_cont1(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = b_cont1(i);
                b_cont1(i) -= total_delta;
            }
        }
        // сброс второй контекстуальной таблицы
        total_delta = 0, table_i, old_b_i_prev = 0;
        if (b_cont2(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = b_cont2(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = b_cont2(i);
                b_cont2(i) -= total_delta;
            }
        }
        c_prev = c;
        context_2[0] = context_2[1];
        context_2[1] = c;
    }
    // cleaning of the buffer
    fwrite(buff_write, 1, (buff_write_pos + BYTE_LEN - 1) / BYTE_LEN, ofp);
    fclose(ifp);
    fclose(ofp);
//    printf("\n");
}

void bits_plus_follow(int bit) {
    write_bit(bit);
    for (; bits_to_follow > 0; bits_to_follow--)
        write_bit(!bit);
}

void decompress_ppm(char *ifile, char *ofile) {
    ifp = (FILE *) fopen(ifile, "rb");
    ofp = (FILE *) fopen(ofile, "wb");

    memset(buff_read, 0, sizeof(buff_read));
    memset(buff_write, 0, sizeof(buff_write));
    buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
    bits_to_follow = 0;

    // definition of aux structures
    interval_t b[N_SYMBOLS + 1];
    for (int i = 0; i <= N_SYMBOLS; i++) {
        b[i] = i;
    }
    interval_t b_cont1[N_SYMBOLS][N_SYMBOLS + 1] = {0};
    for (int i = 0; i < N_SYMBOLS; i++) {
        for (int j = 0; j < N_SYMBOLS + 1; j++) {
            b_cont1[i][j] = j;
        }
    }
    interval_t *b_cont2[N_SYMBOLS][N_SYMBOLS] = {0};

    int n_symbols;
    fread(&n_symbols, sizeof(n_symbols), 1, ifp);
    interval_t val;
    unsigned char buff_help[sizeof(val)] = {0};
    fread(buff_help, 1, 4, ifp);
    unsigned char buff_inv[sizeof(val)] = {buff_help[3], buff_help[2], buff_help[1], buff_help[0], 0, 0, 0, 0};
    val = *((interval_t *) buff_inv);

    interval_t li = 0, hi = 0, li_prev = 0, hi_prev = INTERVAL_LEN, divider = b[N_SYMBOLS];

    int c, c_prev = 0, context_2[2] = {0}; // depends from constants
    int index;

    for (int n_iter = 0; n_iter < n_symbols; n_iter++) {
//        printf("%d\n", n_iter);
        // ищем индекс очередного символа
        divider = sum_b_check(N_SYMBOLS - 1);
//        printf("%lld\n", divider);
        for (index = 0; index < N_SYMBOLS; index++) {
            hi = li_prev + sum_b_check(index) * (hi_prev - li_prev + 1) / divider - 1;
            if (val <= hi) break;
        }
        write_symbol(index);

        if (!b_cont2[context_2[0]][context_2[1]]) {
            b_cont2[context_2[0]][context_2[1]] = calloc(N_SYMBOLS + 1, sizeof(interval_t));
            for (int i = 0; i < N_SYMBOLS; i++) {
                b_cont2(i) = i + 1;
            }
        }

        li = li_prev + sum_b(index - 1) * (hi_prev - li_prev + 1) / divider;
        // обработка вариантов переполнения
        int xxx = 1;
        for (;;) {
            if (hi < HALF) {
            } else if (li >= HALF) {
                val -= HALF;
                li -= HALF;
                hi -= HALF;
            } else if ((li >= FIRST_QTR) && (hi < THIRD_QTR)) {
                val -= FIRST_QTR;
                li -= FIRST_QTR;
                hi -= FIRST_QTR;
            } else
                break;
            li <<= 1;
            hi <<= 1;
            hi += 1;
            int bit = get_bit();
            val <<= 1;
            val += bit;
        }
        li_prev = li, hi_prev = hi;

        // обновление интервалов
        for (int i = index; i < N_SYMBOLS; i++) {
            b(i) += AGGRESSIVENESS;
            b_cont1(i) += AGGRESSIVENESS;
            b_cont2(i) += AGGRESSIVENESS;
        }
        // сброс таблицы
        interval_t total_delta = 0, table_i, old_b_i_prev = b[0];
        if (b(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = b(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = b(i);
                b(i) -= total_delta;
            }
        }
        // сброс контекстуальной таблицы
        total_delta = 0, table_i, old_b_i_prev = b[0];
        if (b_cont1(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = b_cont1(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = b_cont1(i);
                b_cont1(i) -= total_delta;
            }
        }
        total_delta = 0, table_i, old_b_i_prev = 0;
        if (b_cont2(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = b_cont2(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = b_cont2(i);
                b_cont2(i) -= total_delta;
            }
        }
        c = index;
        c_prev = c;
        context_2[0] = context_2[1];
        context_2[1] = c;
    }
    // cleaning of the buffer
    fwrite(buff_write, 1, buff_write_ind, ofp);
    fclose(ifp);
    fclose(ofp);
}

void write_symbol(int symbol) {
    unsigned char c = symbol;
    buff_write[buff_write_ind] = c;
    buff_write_ind++;
    if (buff_write_ind == BUF_LAST_IND) {
        fwrite(buff_write, 1, sizeof(buff_write), ofp);
        memset(buff_write, 0, sizeof(buff_write));
        buff_write_ind = 0;
    }
}

int get_bit() {
    if (buff_read_pos == BUF_LAST_BIT) {
        buff_read_pos = 0;
        memset(buff_read, 0, sizeof(buff_read));
        int n = fread(buff_read, 1, sizeof(buff_read), ifp);
        if (n == 0) {
            return 0;
        }
    }

    unsigned char b = 1 << (BYTE_LEN - buff_read_pos % BYTE_LEN - 1);
    int ans = buff_read[buff_read_pos / BYTE_LEN] & b;
    ans >>= (BYTE_LEN - buff_read_pos % BYTE_LEN - 1);
    buff_read_pos++;
    return ans;
}

int get_symbol() {
    if (buff_read_ind == BUF_LAST_IND) {
        buff_read_ind = 0;
        memset(buff_read, 0, sizeof(buff_read));
        int n = fread(buff_read, 1, sizeof(buff_read), ifp);
        if (n == 0) {
            return -1;
        }
    }
    unsigned int symbol = buff_read[buff_read_ind];
    buff_read_ind++;
    return symbol;
}

void write_bit(int bit) {
    unsigned char b = 1 << (BYTE_LEN - buff_write_pos % BYTE_LEN - 1);
    buff_write[buff_write_pos / BYTE_LEN] |= b * bit;
    buff_write_pos++;
    if (buff_write_pos == BUF_LAST_BIT) {
        fwrite(buff_write, 1, sizeof(buff_write), ofp);
        memset(buff_write, 0, sizeof(buff_write));
        buff_write_pos = 0;
    }
}

