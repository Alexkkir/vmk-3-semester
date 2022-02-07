#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

#include "ppm.h"

//#define b(i) b[i + 1]
//#define b_cont1(i) b_cont1[c_prev][i + 1]
//#define b_cont2(i) b_cont2[context_2[0]][context_2[1]][i + 1]
#define b_cont2_check(i) (b_cont2[context_2[0]][context_2[1]] ? b_cont2[context_2[0]][context_2[1]][i + 1] : i + 1)
// умная (нет) реализация
//#define sum_b(i) (b_cont2(N_SYMBOLS - 1) > (200) ? b_cont2(i) : b_cont1(i))
//#define sum_b_check(i) (b_cont2_check(N_SYMBOLS - 1) > (200) ? b_cont2_check(i) : b_cont1(i))
// прямая реализация
//#define sum_b(i) b_cont2(i)
//#define sum_b_check(i) b_cont2_check(i)
#define sum_b(i) (cur_vert->b[i + 1])
#define sum_b_save(i) (cur_vert ? cur_vert->b[i + 1] : i + 1)

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
    BOR_SIZE = 256 * 256 * 8,
    CONTEXT_LEN = 3,
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

typedef struct vertex {
    int next[N_SYMBOLS];
    int leaf;
    interval_t *b;
} vertex;

void init_bor(vertex *t, int *sz) {
    memset(t[0].next, 255, sizeof t[0].next);
    *sz = 1;
}

vertex *add_elem(vertex *t, int *sz, const int *str, int len) {
    int v = 0;
    for (size_t i = 0; i < len; ++i) {
        int c = str[i]; // в зависимости от алфавита
        if (t[v].next[c] == -1) {
            memset(t[*sz].next, 255, sizeof t[*sz].next);
            t[v].next[c] = (*sz)++;
        }
        v = t[v].next[c];
    }
    t[v].leaf = 1;
    t[v].b = calloc(N_SYMBOLS + 1, sizeof(interval_t));
    if (!t[v].b) {
        printf("out of memory\n");
        exit(1);
    }
    for (int i = 0; i < N_SYMBOLS + 1; i++) {
        t[v].b[i] = i;
    }
    return &t[v];
}

vertex *get_elem(vertex *t, const int *str, int len) {
    int v = 0;
    for (size_t i = 0; i < len; ++i) {
        int c = str[i]; // в зависимости от алфавита
        if (t[v].next[c] == -1) {
            return NULL;
        }
        v = t[v].next[c];
    }
    return &t[v];
}

void compress_ppm(char *ifile, char *ofile) {
    int a = sizeof(size_t);
    ifp = (FILE *) fopen(ifile, "rb");
    ofp = (FILE *) fopen(ofile, "wb");

    memset(buff_read, 0, sizeof(buff_read));
    memset(buff_write, 0, sizeof(buff_write));
    buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
    bits_to_follow = 0;

    // definition of aux structures
//    interval_t b[N_SYMBOLS + 1];
//    for (int i = 0; i <= N_SYMBOLS; i++) {
//        b[i] = i;
//    }
//
//    interval_t b_cont1[N_SYMBOLS][N_SYMBOLS + 1] = {0};
//    for (int i = 0; i < N_SYMBOLS; i++) {
//        for (int j = 0; j < N_SYMBOLS + 1; j++) {
//            b_cont1[i][j] = j;
//        }
//    }
//
//    interval_t *b_cont2[N_SYMBOLS][N_SYMBOLS] = {0};

    vertex *bor = ((vertex*) calloc(BOR_SIZE, sizeof(vertex)));
    int bor_size;
    init_bor(bor, &bor_size);

    fseek(ifp, 0, SEEK_END);
    int n_symbols = ftell(ifp);
    fwrite(&n_symbols, sizeof(n_symbols), 1, ofp);
    fseek(ifp, 0, SEEK_SET);

    interval_t li = 0, hi = 0, li_prev = 0, hi_prev = INTERVAL_LEN, divider;
    int add_to_end = ADD_TO_END, max_freq_ind, found = 0;
    interval_t max_freq;

//    int c_prev = 0, context_2[2] = {0}; // depends on constants
    int c;
    int context[CONTEXT_LEN] = {0};
    int index;
    int n_iter;
    for (n_iter = 0;; n_iter++) { // основной цикл
//        printf("%d\n", n_iter);
        vertex *cur_vert = NULL;
        if (get_elem(bor, context, CONTEXT_LEN) == NULL) {
            cur_vert = add_elem(bor, &bor_size, context, CONTEXT_LEN);
        } else {
            cur_vert = get_elem(bor, context, CONTEXT_LEN);
        }
        c = get_symbol(); // Читаем символ

        if (c == -1 || n_iter >= n_symbols) {
            if (add_to_end > 0) {
                if (found == 0) {
                    found = 1, max_freq = 0, max_freq_ind = 0;
                    for (int i = 0; i < N_SYMBOLS; i++) {
                        interval_t freq = sum_b(i) - sum_b(i - 1);
                        if (freq > max_freq) {
                            max_freq = freq;
                            max_freq_ind = i;
                        }
                    }
                    index = max_freq_ind;
                    c = max_freq_ind;
                }
                add_to_end--;
            } else {
                break;
            }
        } else {
            index = c;
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
            sum_b(i) += AGGRESSIVENESS;
//            b_cont1(i) += AGGRESSIVENESS;
//            b_cont2(i) += AGGRESSIVENESS;
        }
//         сброс таблицы
        interval_t total_delta, table_i, old_b_i_prev;
        total_delta = 0, table_i, old_b_i_prev = 0;
        if (sum_b(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = sum_b(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = sum_b(i);
                sum_b(i) -= total_delta;
            }
        }
//        // сброс контекстуальной таблицы
//        total_delta = 0, table_i, old_b_i_prev = 0;
//        if (b_cont1(N_SYMBOLS - 1) >= THRESHOLD) {
//            for (int i = 0; i < N_SYMBOLS; i++) {
//                table_i = b_cont1(i) - old_b_i_prev;
//                total_delta += ((table_i + 1) >> 1) - 1;
//                old_b_i_prev = b_cont1(i);
//                b_cont1(i) -= total_delta;
//            }
//        }
//        // сброс второй контекстуальной таблицы
//        total_delta = 0, table_i, old_b_i_prev = 0;
//        if (b_cont2(N_SYMBOLS - 1) >= THRESHOLD) {
//            for (int i = 0; i < N_SYMBOLS; i++) {
//                table_i = b_cont2(i) - old_b_i_prev;
//                total_delta += ((table_i + 1) >> 1) - 1;
//                old_b_i_prev = b_cont2(i);
//                b_cont2(i) -= total_delta;
//            }
//        }
//        c_prev = c;
//        context_2[0] = context_2[1];
//        context_2[1] = c;
        for (int i = 0; i < CONTEXT_LEN - 1; i++) {
            context[i] = context[i + 1];
        }
        context[CONTEXT_LEN - 1] = c;
        if (n_iter < n_symbols) {
//            printf("%lld\n", divider);
        }
    }
    // cleaning of the buffer
    fwrite(buff_write, 1, (buff_write_pos + BYTE_LEN - 1) / BYTE_LEN, ofp);
    fclose(ifp);
    fclose(ofp);
    printf("\n");
}

void decompress_ppm(char *ifile, char *ofile) {
    ifp = (FILE *) fopen(ifile, "rb");
    ofp = (FILE *) fopen(ofile, "wb");

    memset(buff_read, 0, sizeof(buff_read));
    memset(buff_write, 0, sizeof(buff_write));
    buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
    bits_to_follow = 0;

    // definition of aux structures
//    interval_t b[N_SYMBOLS + 1];
//    for (int i = 0; i <= N_SYMBOLS; i++) {
//        b[i] = i;
//    }
//    interval_t b_cont1[N_SYMBOLS][N_SYMBOLS + 1] = {0};
//    for (int i = 0; i < N_SYMBOLS; i++) {
//        for (int j = 0; j < N_SYMBOLS + 1; j++) {
//            b_cont1[i][j] = j;
//        }
//    }
//    interval_t *b_cont2[N_SYMBOLS][N_SYMBOLS] = {0};

    vertex *bor = ((vertex*) calloc(BOR_SIZE, sizeof(vertex)));
    int bor_size;
    init_bor(bor, &bor_size);

    int n_symbols;
    fread(&n_symbols, sizeof(n_symbols), 1, ifp);
    interval_t val;
    unsigned char buff_help[sizeof(val)] = {0};
    fread(buff_help, 1, 4, ifp);
    unsigned char buff_inv[sizeof(val)] = {buff_help[3], buff_help[2], buff_help[1], buff_help[0], 0, 0, 0, 0};
    val = *((interval_t *) buff_inv);

    interval_t li = 0, hi = 0, li_prev = 0, hi_prev = INTERVAL_LEN, divider;

//    int c, c_prev = 0, context_2[2] = {0}; // depends from constants
    int c;
    int context[CONTEXT_LEN] = {0};
    int index;

    for (int n_iter = 0; n_iter < n_symbols; n_iter++) {
//        printf("%d\n", n_iter);
        // ищем индекс очередного символа
        vertex *cur_vert = NULL;
        if (!get_elem(bor, context, CONTEXT_LEN)) {
            cur_vert = add_elem(bor, &bor_size, context, CONTEXT_LEN);
        } else {
            cur_vert = get_elem(bor, context, CONTEXT_LEN);
        }

        divider = sum_b(N_SYMBOLS - 1);
        for (index = 0; index < N_SYMBOLS; index++) {
            hi = li_prev + sum_b(index) * (hi_prev - li_prev + 1) / divider - 1;
            if (val <= hi) break;
        }
        write_symbol(index);
        c = index;

//        if (!b_cont2[context_2[0]][context_2[1]]) {
//            b_cont2[context_2[0]][context_2[1]] = calloc(N_SYMBOLS + 1, sizeof(interval_t));
//            for (int i = 0; i < N_SYMBOLS; i++) {
//                b_cont2(i) = i + 1;
//            }
//        }

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
            sum_b(i) += AGGRESSIVENESS;
//            b_cont1(i) += AGGRESSIVENESS;
//            b_cont2(i) += AGGRESSIVENESS;
        }
        // сброс таблицы
        interval_t total_delta = 0, table_i, old_b_i_prev = 0;
        if (sum_b(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = sum_b(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = sum_b(i);
                sum_b(i) -= total_delta;
            }
        }
//        // сброс контекстуальной таблицы
//        total_delta = 0, table_i, old_b_i_prev = b[0];
//        if (b_cont1(N_SYMBOLS - 1) >= THRESHOLD) {
//            for (int i = 0; i < N_SYMBOLS; i++) {
//                table_i = b_cont1(i) - old_b_i_prev;
//                total_delta += ((table_i + 1) >> 1) - 1;
//                old_b_i_prev = b_cont1(i);
//                b_cont1(i) -= total_delta;
//            }
//        }
//        total_delta = 0, table_i, old_b_i_prev = 0;
//        if (b_cont2(N_SYMBOLS - 1) >= THRESHOLD) {
//            for (int i = 0; i < N_SYMBOLS; i++) {
//                table_i = b_cont2(i) - old_b_i_prev;
//                total_delta += ((table_i + 1) >> 1) - 1;
//                old_b_i_prev = b_cont2(i);
//                b_cont2(i) -= total_delta;
//            }
//        }
//        c = index;
//        c_prev = c;
//        context_2[0] = context_2[1];
//        context_2[1] = c;

        for (int i = 0; i < CONTEXT_LEN - 1; i++) {
            context[i] = context[i + 1];
        }
        context[CONTEXT_LEN - 1] = c;
//        printf("%lld\n", divider);
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

void bits_plus_follow(int bit) {
    write_bit(bit);
    for (; bits_to_follow > 0; bits_to_follow--)
        write_bit(!bit);
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

