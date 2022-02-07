#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

#include "ppm.h"

#define max(a, b) a > b ? a : b
#define min (a, b) a < b ? a : b

#define total_b(i) (cur_verts[best_order]->tab.b[i + 1])

#define abs(x) ((x) >= 0 ? (x) : (-x))

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
    SKIP_TH = 256 + 200 + 50,
    RESET_N = 100,
    BAD_CODE_DELTA = 3000,
};

static int RESCALE_1 = 8192 * 3, AGGR_1 = 100;
static int RESCALE_2 = 8192 / 2, AGGR_2 = 150;
static unsigned char buff_write[BUF_LEN] = {0}, buff_read[BUF_LEN] = {0};
static int buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
static int bits_to_follow = 0;
static FILE *ifp, *ofp;
double MAX_DIFF = 0.05;

static void write_bit(int bit);

typedef struct Table {
    interval_t *b, *b_2;
} Table;

static void bits_plus_follow(int bit);

static int get_bit();

static int get_symbol();

static void write_symbol(int symbol);

void init_tab(interval_t *b) {
    for (int i = 0; i <= N_SYMBOLS; i++) {
        b[i] = i;
    }
}

typedef struct Vertex {
    int next[N_SYMBOLS];
    Table tab;
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
    t[v].tab.b = calloc(N_SYMBOLS + 1, sizeof(interval_t));
    t[v].tab.b_2 = calloc(N_SYMBOLS + 1, sizeof(interval_t));
    if (!t[v].tab.b || !t[v].tab.b_2) {
        printf("out of memory\n");
        exit(1);
    }
    init_tab(t[v].tab.b);
    init_tab(t[v].tab.b_2);
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

void update_tab(interval_t *b, int index, interval_t aggr) {
    for (int i = index + 1; i <= N_SYMBOLS; i++) {
        b[i] += aggr;
    }
}

void rescale_tab(interval_t *b, interval_t rescale_th) {
    if (b[N_SYMBOLS] > rescale_th) {
        interval_t total_delta, old_b_i_prev, table_i;
        total_delta = 0, old_b_i_prev = 0;
        for (int i = 1; i <= N_SYMBOLS; i++) {
            table_i = b[i] - old_b_i_prev;
            total_delta += ((table_i + 1) >> 1) - 1;
            old_b_i_prev = b[i];
            b[i] -= total_delta;
        }
    }
}

double diff_tab(const interval_t *b1, const interval_t *b2) {
    double norm1 = (double) b1[N_SYMBOLS], norm2 = (double) b2[N_SYMBOLS];
    double tot = 0;
    for (int i = 1; i <= N_SYMBOLS; i++) {
        interval_t f1 = b1[i] - b1[i - 1], f2 = b2[i] - b2[i - 1];
        double delta = f1 * norm2 - f2 * norm1;
        tot += abs(delta);
    }
    tot /= norm1 * norm2;
    return tot;
}

void delete_bor(Vertex *bor, int bor_size) {
    for (int i = 0; i < bor_size; i++) {
        free(bor[i].tab.b);
        free(bor[i].tab.b_2);
    }
    free(bor);
}

void update_context(int *context, int c) {
    for (int i = 0; i < CONTEXT_LEN - 1; i++) {
        context[i] = context[i + 1];
    }
    context[CONTEXT_LEN - 1] = c;
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

    interval_t li = 0, hi = 0, li_prev = 0, hi_prev = INTERVAL_LEN, divider;
    int add_to_end = ADD_TO_END, max_freq_ind, found = 0;
    interval_t max_freq;

    int c;
    int context[CONTEXT_LEN] = {0};
    int index;
    int n_iter;

    int cl_prev = 0, cl_cur = 0;

    int f_blank_work = 0;
    for (n_iter = 0;; n_iter++) { // основной цикл
        Vertex *cur_verts[CONTEXT_LEN] = {0};
        int max_order = 0;
        for (int i = 0; i < CONTEXT_LEN; i++) {
            Vertex *cur_vert = NULL;
            if ((cur_vert = get_elem(bor, context + CONTEXT_LEN - i - 1, i + 1)) == NULL || cur_vert->tab.b == NULL) {
                // вершина отсутствует ИЛИ вершина присутсвует, но она пустая
                cur_vert = add_elem(bor, &bor_size, context + CONTEXT_LEN - i - 1, i + 1);
            } else {
                // вершина присутсвует и из нее торчит таблица
                max_order = i;
            }
            cur_verts[i] = cur_vert;
        }

        c = get_symbol(); // Читаем символ

        int best_order = max_order;
        while (best_order > 0 && cur_verts[best_order]->tab.b[N_SYMBOLS] < SKIP_TH) {
            best_order--;
        }

        if (c == -1 || n_iter >= n_symbols) {
            if (add_to_end > 0) {
                if (found == 0) {
                    found = 1, max_freq = 0, max_freq_ind = 0;
                    for (int i = 0; i < N_SYMBOLS; i++) {
                        interval_t freq = total_b(i) - total_b(i - 1);
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

        divider = total_b(N_SYMBOLS - 1);
        li = li_prev + total_b(index - 1) * (hi_prev - li_prev + 1) / divider;
        hi = li_prev + total_b(index) * (hi_prev - li_prev + 1) / divider - 1;
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
            cl_cur++;
        }
        li_prev = li, hi_prev = hi;

        if (n_iter % RESET_N == 0 && n_iter < n_symbols) {
            double diff = diff_tab(cur_verts[best_order]->tab.b, cur_verts[best_order]->tab.b_2);
//            printf("%f\n", diff);
            printf("%d\n", cl_cur - cl_prev);
            if (cl_cur - cl_prev > BAD_CODE_DELTA) {
                for (int j = 0; j < CONTEXT_LEN; j++) {
//                    rescale_tab(cur_verts[j]->tab.b, 0);
//                    rescale_tab(cur_verts[j]->tab.b_2, 0);
//                    rescale_tab(cur_verts[j]->tab.b, 0);
//                    rescale_tab(cur_verts[j]->tab.b_2, 0);
                    init_tab(cur_verts[j]->tab.b);
                    init_tab(cur_verts[j]->tab.b_2);
                }
            }
            cl_prev = cl_cur;
            cl_cur = 0;
        }

        // обновление интервалов
        for (int j = 0; j < CONTEXT_LEN; j++) {
            update_tab(cur_verts[j]->tab.b, index, AGGR_1);
            update_tab(cur_verts[j]->tab.b_2, index, AGGR_2);
        }
 //         сброс таблицы
        for (int j = 0; j < CONTEXT_LEN; j++) {
            rescale_tab(cur_verts[j]->tab.b, RESCALE_1);
            rescale_tab(cur_verts[j]->tab.b_2, RESCALE_2);
        }

        update_context(context, c);
    }
    // cleaning of the buffer
    printf("-------------end\n");
    delete_bor(bor, bor_size);
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

    interval_t li = 0, hi = 0, li_prev = 0, hi_prev = INTERVAL_LEN, divider;

//    int c, c_prev = 0, context_2[2] = {0}; // depends from constants
    int c;
    int context[CONTEXT_LEN] = {0};
    int index;

    int cl_prev = 0, cl_cur = 0;
    for (int n_iter = 0; n_iter < n_symbols; n_iter++) {
        // ищем индекс очередного символа
        Vertex *cur_verts[CONTEXT_LEN] = {0};
        int max_order = 0;
        for (int i = 0; i < CONTEXT_LEN; i++) {
            Vertex *cur_vert = NULL;
            if ((cur_vert = get_elem(bor, context + CONTEXT_LEN - i - 1, i + 1)) == NULL || cur_vert->tab.b == NULL) {
                // вершина отсутствует ИЛИ вершина присутсвует, но она пустая
                cur_vert = add_elem(bor, &bor_size, context + CONTEXT_LEN - i - 1, i + 1);
            } else {
                // вершина присутсвует и из нее торчит таблица
                max_order = i;
            }
            cur_verts[i] = cur_vert;
        }

        int best_order = max_order;
        while (best_order > 0 && cur_verts[best_order]->tab.b[N_SYMBOLS] < SKIP_TH) {
            best_order--;
        }

        divider = total_b(N_SYMBOLS - 1);
        for (index = 0; index < N_SYMBOLS; index++) {
            hi = li_prev + total_b(index) * (hi_prev - li_prev + 1) / divider - 1;
            if (val <= hi) break;
        }

        write_symbol(index);
        c = index;

        divider = total_b(N_SYMBOLS - 1);
        li = li_prev + total_b(index - 1) * (hi_prev - li_prev + 1) / divider;
        hi = li_prev + total_b(index) * (hi_prev - li_prev + 1) / divider -
             1;        // обработка вариантов переполнения
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
            cl_cur++;
        }
        li_prev = li, hi_prev = hi;

        if (n_iter % RESET_N == 0 && n_iter < n_symbols) {
            double diff = diff_tab(cur_verts[best_order]->tab.b, cur_verts[best_order]->tab.b_2);
//            printf("%f\n", diff);
            if (cl_cur - cl_prev > BAD_CODE_DELTA) {
                for (int j = 0; j < CONTEXT_LEN; j++) {
//                    rescale_tab(cur_verts[j]->tab.b, 0);
//                    rescale_tab(cur_verts[j]->tab.b_2, 0);
//                    rescale_tab(cur_verts[j]->tab.b, 0);
//                    rescale_tab(cur_verts[j]->tab.b_2, 0);
                    init_tab(cur_verts[j]->tab.b);
                    init_tab(cur_verts[j]->tab.b_2);
                }
            }
            cl_prev = cl_cur;
            cl_cur = 0;
        }

        // обновление интервалов
        for (int j = 0; j < CONTEXT_LEN; j++) {
            update_tab(cur_verts[j]->tab.b, index, AGGR_1);
            update_tab(cur_verts[j]->tab.b_2, index, AGGR_2);
        }
        //         сброс таблицы
        for (int j = 0; j < CONTEXT_LEN; j++) {
            rescale_tab(cur_verts[j]->tab.b, RESCALE_1);
            rescale_tab(cur_verts[j]->tab.b_2, RESCALE_2);
        }

        update_context(context, c);
    }

    // cleaning of the buffer
    delete_bor(bor, bor_size);
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