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
//#define total_b(i) (b_cont2(N_SYMBOLS - 1) > (200) ? b_cont2(i) : b_cont1(i))
//#define sum_b_check(i) (b_cont2_check(N_SYMBOLS - 1) > (200) ? b_cont2_check(i) : b_cont1(i))
// прямая реализация
//#define total_b(i) b_cont2(i)
//#define sum_b_check(i) b_cont2_check(i)
#define total_b(i) (cur_verts[best_order]->b[i + 1])
#define sum_b_save(i) (cur_vert ? cur_vert->b[i + 1] : i + 1)

#define max(a, b) (a > b ? a : b)
#define min(a, b) (a < b ? a : b)
#define abs(x) (x >= 0 ? x : -x)

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
    BOR_SIZE = 256 * 256 * 64,
    CONTEXT_LEN = 3,
    SKIP_TH = 256 + 200 + 50,
    CHUNK_LEN = 128,
};

static int THRESHOLD = 8192 * 3, AGGRESSIVENESS = 100;
static unsigned char buff_write[BUF_LEN] = {0}, buff_read[BUF_LEN] = {0}, buff_interm[CHUNK_LEN * 3] = {0};
static int buff_write_pos = 0, buff_write_ind = 0,
        buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT,
        buff_interm_pos = 0;
static int bits_to_follow = 0;
static FILE *ifp, *ofp;

static void write_bit(int bit);

static void bits_plus_follow(int bit);

static int get_bit();

static int get_symbol();

static void write_symbol(int symbol);

typedef struct Vertex {
    int next[N_SYMBOLS];
    interval_t *b;
} Vertex;

void init_bor(Vertex *t, int *sz) {
    memset(t[0].next, 255, sizeof t[0].next);
    *sz = 1;
}

Vertex *add_elem(Vertex *t, int *sz, const int *str, int len) {
    int v = 0;
    for (size_t i = 0; i < len; ++i) {
        int c = str[i]; // в зависимости от алфавита
        if (t[v].next[c] == -1) {
            memset(t[*sz].next, 255, sizeof t[*sz].next);
            t[v].next[c] = (*sz)++;
        }
        v = t[v].next[c];
    }
//    t[v].leaf = 1;
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

Vertex *get_elem(Vertex *t, const int *str, int len) {
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

//void normalize(interval_t *p_left, interval_t *p_right, interval_t *p_val, int f_compress, int f_blank_work) {
//    interval_t left = *p_left, right = *p_right, val = !f_compress ? *p_val : 0;
//    for (;;) {
//        if (right < HALF) {
//            if (!f_blank_work)
//                bits_plus_follow(0);
//        } else if (left >= HALF) {
//            if (!f_blank_work)
//                bits_plus_follow(1);
//            left -= HALF;
//            right -= HALF;
//        } else if ((left >= FIRST_QTR) && (right < THIRD_QTR)) {
//            if (!f_blank_work)
//                bits_to_follow++;
//            left -= FIRST_QTR;
//            right -= FIRST_QTR;
//        } else
//            break;
//        left <<= 1;
//        right <<= 1;
//        right += 1;
//    }
//    *p_left = left, *p_right = right;
//    if (!f_compress) {
//        *p_val = val;
//    }
//}

void process_n_symbols(Vertex *bor, int *p_bor_size,
                       int n_symbols,
                       interval_t *p_left, interval_t *p_right,
                       interval_t *p_left_prev, interval_t *p_right_prev,
                       interval_t *p_val,
                       int *context,
                       int f_compress,
                       int f_blank_work) {
    int bor_size = *p_bor_size;
    interval_t left = *p_left, right = *p_right,
            left_prev = *p_left_prev, right_prev = *p_right_prev, val = 0;
    if (!f_compress) {
        val = *p_val;
    }

    int add_to_end = ADD_TO_END, max_freq_ind, found = 0;
    interval_t max_freq;

    int c;
    int index;
    int n_iter;

    for (n_iter = 0; n_iter < n_symbols; n_iter++) { // основной цикл
        Vertex *cur_verts[CONTEXT_LEN] = {0};
        int max_order = 0;
        for (int i = 0; i < CONTEXT_LEN; i++) {
            Vertex *cur_vert = NULL;
            if ((cur_vert = get_elem(bor, context + CONTEXT_LEN - i - 1, i + 1)) == NULL || cur_vert->b == NULL) {
                // вершина отсутствует ИЛИ вершина присутсвует, но она пустая
                cur_vert = add_elem(bor, &bor_size, context + CONTEXT_LEN - i - 1, i + 1);
            } else {
                // вершина присутсвует и из нее торчит таблица
                max_order = i;
            }
            cur_verts[i] = cur_vert;
        }

        int best_order = max_order;
        while (best_order > 0 && cur_verts[best_order]->b[N_SYMBOLS] < SKIP_TH) {
            best_order--;
        }

        if (f_compress) {
            c = get_symbol();
            index = c;
        } else {
            interval_t divider = total_b(N_SYMBOLS - 1);
            for (index = 0; index < N_SYMBOLS; index++) {
                right = left_prev + total_b(index) * (right_prev - left_prev + 1) / divider - 1;
                if (val <= right) break;
            }

            write_symbol(index);
            c = index;
        }

        interval_t divider = total_b(N_SYMBOLS - 1);
        left = left_prev + total_b(index - 1) * (right_prev - left_prev + 1) / divider;
        right = left_prev + total_b(index) * (right_prev - left_prev + 1) / divider - 1;
        // обработка вариантов переполнения
        int xxx = 1;
        for (;;) {
            if (right < HALF) {
                if (f_compress && !f_blank_work) {
                    bits_plus_follow(0);
                }
            } else if (left >= HALF) {
                if (f_compress && !f_blank_work) {
                    bits_plus_follow(1);
                }
                left -= HALF;
                right -= HALF;
                if (!f_compress) {
                    val -= HALF;
                }
            } else if ((left >= FIRST_QTR) && (right < THIRD_QTR)) {
                if (f_compress && !f_blank_work) {
                    bits_to_follow++;
                }
                left -= FIRST_QTR;
                right -= FIRST_QTR;
                if (!f_compress) {
                    val -= FIRST_QTR;
                }
            } else
                break;
            left <<= 1;
            right <<= 1;
            right += 1;
            if (!f_compress) {
                int bit = get_bit();
                val <<= 1;
                val += bit;
            }
        }
        left_prev = left, right_prev = right;

        // обновление интервалов
        for (int i = index; i < N_SYMBOLS; i++) {
            for (int j = 0; j < CONTEXT_LEN; j++) {
                cur_verts[j]->b[i + 1] += AGGRESSIVENESS;
            }
        }
//         сброс таблицы
        interval_t total_delta, table_i, old_b_i_prev;
        total_delta = 0, table_i, old_b_i_prev = 0;
        if (total_b(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = total_b(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = total_b(i);
                total_b(i) -= total_delta;
            }
        }
        for (int i = 0; i < CONTEXT_LEN - 1; i++) {
            context[i] = context[i + 1];
        }
        context[CONTEXT_LEN - 1] = c;
    }

    *p_bor_size = bor_size,
    *p_left = left, *p_right = right,
    *p_left_prev = left_prev, *p_right_prev = right_prev;
    if (!f_compress) {
        *p_val = val;
    }
}

void compress_ppm(char *ifile, char *ofile) {
    int a = sizeof(size_t);
    ifp = (FILE *) fopen(ifile, "rb");
    ofp = (FILE *) fopen(ofile, "wb");

    memset(buff_read, 0, sizeof(buff_read));
    memset(buff_write, 0, sizeof(buff_write));
    buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
    bits_to_follow = 0;

    Vertex *bor = ((Vertex *) calloc(BOR_SIZE, sizeof(Vertex)));
    int bor_size;
    init_bor(bor, &bor_size);

    fseek(ifp, 0, SEEK_END);
    int n_symbols = ftell(ifp);
    fwrite(&n_symbols, sizeof(n_symbols), 1, ofp);
    fseek(ifp, 0, SEEK_SET);

    interval_t left = 0, right = 0, left_prev = 0, right_prev = INTERVAL_LEN, divider;
    int add_to_end = ADD_TO_END, max_freq_ind, found = 0;
    interval_t max_freq;

    int c;
    int context[CONTEXT_LEN] = {0};
    int index;
    int n_iter;

    for (n_iter = 0; n_iter < n_symbols; n_iter += CHUNK_LEN) { // основной цикл
        process_n_symbols(bor, &bor_size, CHUNK_LEN, &left, &right, &left_prev, &right_prev, NULL, context, 1, 0);
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

    Vertex *bor = ((Vertex *) calloc(BOR_SIZE, sizeof(Vertex)));
    int bor_size;
    init_bor(bor, &bor_size);

    int n_symbols;
    fread(&n_symbols, sizeof(n_symbols), 1, ifp);
    interval_t val;
    unsigned char buff_help[sizeof(val)] = {0};
    fread(buff_help, 1, 4, ifp);
    unsigned char buff_inv[sizeof(val)] = {buff_help[3], buff_help[2], buff_help[1], buff_help[0], 0, 0, 0, 0};
    val = *((interval_t *) buff_inv);

    interval_t left = 0, right = 0, left_prev = 0, right_prev = INTERVAL_LEN, divider;

//    int c, c_prev = 0, context_2[2] = {0}; // depends from constants
    int c;
    int context[CONTEXT_LEN] = {0};
    int index;

    for (int n_iter = 0; n_iter < n_symbols; n_iter++) {
        // ищем индекс очередного символа
        Vertex *cur_verts[CONTEXT_LEN] = {0};
        int max_order = 0;
        for (int i = 0; i < CONTEXT_LEN; i++) {
            Vertex *cur_vert = NULL;
            if ((cur_vert = get_elem(bor, context + CONTEXT_LEN - i - 1, i + 1)) == NULL || cur_vert->b == NULL) {
                // вершина отсутствует ИЛИ вершина присутсвует, но она пустая
                cur_vert = add_elem(bor, &bor_size, context + CONTEXT_LEN - i - 1, i + 1);
            } else {
                // вершина присутсвует и из нее торчит таблица
                max_order = i;
            }
            cur_verts[i] = cur_vert;
        }

        int best_order = max_order;
        while(best_order > 0 && cur_verts[best_order]->b[N_SYMBOLS] < SKIP_TH)
        {
            best_order--;
        }

        divider = total_b(N_SYMBOLS - 1);
        for (index = 0; index < N_SYMBOLS; index++) {
            right = left_prev + total_b(index) * (right_prev - left_prev + 1) / divider - 1;
            if (val <= right) break;
        }

        write_symbol(index);
        c = index;

        divider = total_b(N_SYMBOLS - 1);
        left = left_prev + total_b(index - 1) * (right_prev - left_prev + 1) / divider;
        right = left_prev + total_b(index) * (right_prev - left_prev + 1) / divider -
                1;        // обработка вариантов переполнения
        int xxx = 1;
        for (;;) {
            if (right < HALF) {
            } else if (left >= HALF) {
                val -= HALF;
                left -= HALF;
                right -= HALF;
            } else if ((left >= FIRST_QTR) && (right < THIRD_QTR)) {
                val -= FIRST_QTR;
                left -= FIRST_QTR;
                right -= FIRST_QTR;
            } else
                break;
            left <<= 1;
            right <<= 1;
            right += 1;
            int bit = get_bit();
            val <<= 1;
            val += bit;
        }
        left_prev = left, right_prev = right;

        // обновление интервалов
        for (int i = index; i < N_SYMBOLS; i++) {
            for (int j = 0; j < CONTEXT_LEN; j++) {
                cur_verts[j]->b[i + 1] += AGGRESSIVENESS;
            }
        }
        // сброс таблицы
        interval_t total_delta = 0, table_i, old_b_i_prev = 0;
        if (total_b(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = total_b(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = total_b(i);
                total_b(i) -= total_delta;
            }
        }
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