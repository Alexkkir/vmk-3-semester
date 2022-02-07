#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <limits.h>

#include "ppm.h"

#define context(len) (context + CONTEXT_LEN - len)
#define DO(var, n) for (int var = 0; var < n; var++)

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
    MAX_BOR_SIZE = 256 * 256 * 4,
    CONTEXT_LEN = 3,
    SKIP_TH = 200 + 50,
};

typedef struct {
    interval_t *freq, sum, esc;
} Table;

typedef struct {
    int next[N_SYMBOLS + 1];
    Table table;
} Vertex;

static int MAX_TOTAL_SUM = 8192 * 3, AGGRESSIVENESS = 100;
static unsigned char buff_write[BUF_LEN] = {0}, buff_read[BUF_LEN] = {0};
static int buff_write_pos = 0, buff_write_ind = 0,
        buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT,
        bits_to_follow = 0,
        bor_size, SP;
static Vertex *bor;
static Table *stack[CONTEXT_LEN];
static FILE *ifp, *ofp;

void
delete_bor();

Table
new_table();

Vertex *
add_elem(const int *str, int len);

Vertex *
get_elem(const int *str, int len);

void
init_bor_and_stack();

static void
write_bit(int bit);

static void
bits_plus_follow(int bit);

static int
get_bit();

static int
get_symbol();

static void
write_symbol(int symbol);

static int
get_filesize(FILE *f);

static void
setup_buffers();

void
rescale(Table *table);

void
update_model(int c);

void
normalize_and_write(interval_t *p_left, interval_t *p_right);

int
get_interval(Table *table, int c, interval_t *left, interval_t *right, interval_t *divider);

void update_context(int context[CONTEXT_LEN], int c);

int encode_sym(Table *p_table, int c, interval_t *p_left_prev, interval_t *p_right_prev,
               interval_t *p_left, interval_t *p_right);

void compress_ppm(char *ifile, char *ofile) {
    ifp = (FILE *) fopen(ifile, "rb");
    ofp = (FILE *) fopen(ofile, "wb");
    setup_buffers();
    init_bor_and_stack();
    int n_symbols = get_filesize(ifp);
    fwrite(&n_symbols, sizeof(n_symbols), 1, ofp);

    // variables for main loop
    interval_t left = 0, right = 0, left_prev = 0, right_prev = INTERVAL_LEN, divider;
//	int add_to_end = ADD_TO_END, max_freq_ind, found = 0;
//	interval_t max_freq;
    int c;
    int context[CONTEXT_LEN] = {0};
    int n_iter;

    // main loop
    for (n_iter = 0; n_iter <= n_symbols; n_iter++) { // основной цикл
        // reading a symbol
        c = get_symbol();
        // what about end???

        Vertex *elem = NULL;
        Table *cur_table = NULL;
        int len = CONTEXT_LEN, success = 0;
        while (!success) {
            elem = get_elem(context(len), len);
            if (!elem) {
                // creat and add to stack
                cur_table = &add_elem(context(len), len)->table;
                stack[++SP] = cur_table;
            } else {
                // context exists
                cur_table = &elem->table;
                success = encode_sym(cur_table, c, &left_prev, &right_prev, &left, &right);
                if (!success) {
                    stack[++SP] = cur_table;
                }
            }
        }

        update_model(c);
        update_context(context, c);
    }

    // cleaning of the buffer
    fwrite(buff_write, 1, (buff_write_pos + BYTE_LEN - 1) / BYTE_LEN, ofp);
    delete_bor();
    fclose(ifp);
    fclose(ofp);
    printf("\n");
}

void
decompress_ppm(char *ifile, char *ofile) {
    ifp = (FILE *) fopen(ifile, "rb");
    ofp = (FILE *) fopen(ofile, "wb");

    setup_buffers();
    init_bor_and_stack();
    int n_symbols;
    fread(&n_symbols, sizeof(n_symbols), 1, ifp);

    // reading val
    interval_t val;
    unsigned char buff_help[sizeof(val)] = {0};
    fread(buff_help, 1, 4, ifp);
    unsigned char buff_inv[sizeof(val)] = {buff_help[3], buff_help[2], buff_help[1], buff_help[0], 0, 0, 0, 0};
    val = *((interval_t *) buff_inv);

    interval_t left = 0, right = 0, left_prev = 0, right_prev = INTERVAL_LEN, divider;

    int c;
    int context[CONTEXT_LEN] = {0};
    int index;

    for (int n_iter = 0; n_iter < n_symbols; n_iter++) {
        // ищем индекс очередного символа


        for (index = 0; index < N_SYMBOLS; index++) {
            best_order = max_order;
            while (best_order > 0 && cur_verts[best_order]->b[c + 1] - cur_verts[best_order]->b[c] < SKIP_TH) {
                best_order--;
            }
            divider = total_b(N_SYMBOLS - 1);
            right = left_prev + total_b(index) * (right_prev - left_prev + 1) / divider - 1;
            if (val <= right)
                break;
        }

        write_symbol(index);
        c = index;

        divider = total_b(N_SYMBOLS - 1);
        left = left_prev + total_b(index - 1) * (right_prev - left_prev + 1) / divider;
        right =
                left_prev + total_b(index) * (right_prev - left_prev + 1) / divider
                - 1;        // обработка вариантов переполнения
        int xxx = 1;

        // normalize

        left_prev = left, right_prev = right;

        // обновление интервалов
        update_model(c);
        update_context(context, c);
    }
    // cleaning of the buffer
    fwrite(buff_write, 1, buff_write_ind, ofp);
    delete_bor();
    fclose(ifp);
    fclose(ofp);
}

/*================ READ/WRITE FUNCTIONS ================*/

void
write_symbol(int symbol) {
    unsigned char c = symbol;
    buff_write[buff_write_ind] = c;
    buff_write_ind++;
    if (buff_write_ind == BUF_LAST_IND) {
        fwrite(buff_write, 1, sizeof(buff_write), ofp);
        memset(buff_write, 0, sizeof(buff_write));
        buff_write_ind = 0;
    }
}

void
bits_plus_follow(int bit) {
    write_bit(bit);
    for (; bits_to_follow > 0; bits_to_follow--)
        write_bit(!bit);
}

int
get_bit() {
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

int
get_symbol() {
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

void
write_bit(int bit) {
    unsigned char b = 1 << (BYTE_LEN - buff_write_pos % BYTE_LEN - 1);
    buff_write[buff_write_pos / BYTE_LEN] |= b * bit;
    buff_write_pos++;
    if (buff_write_pos == BUF_LAST_BIT) {
        fwrite(buff_write, 1, sizeof(buff_write), ofp);
        memset(buff_write, 0, sizeof(buff_write));
        buff_write_pos = 0;
    }
}

/*================ BOR, TABLE, STACK API FUNCTIONS ================*/

void
delete_bor() {
    for (int i = 0; i < bor_size; i++) {
        free(bor[i].table.freq);
    }
    free(bor);
}

Table
new_table() {
    Table table;
    table.esc = 0;
    table.sum = 0;
    table.freq = calloc(N_SYMBOLS + 1, sizeof(table.freq[0]));
}

Vertex *
add_elem(const int *str, int len) {
    int v = 0;
    for (size_t i = 0; i < len; ++i) {
        int c = str[i]; // в зависимости от алфавита
        if (bor[v].next[c] == -1) {
            memset(bor[bor_size].next, 255, sizeof bor[bor_size].next);
            bor[v].next[c] = bor_size++;
        }
        v = bor[v].next[c];
    }
    // assumed that leaf is empty
    bor[v].table = new_table();
    if (!bor[v].table.freq) {
        printf("out of memory\n");
        exit(1);
    }
    return &bor[v];
}

Vertex *
get_elem(const int *str, int len) {
    if (len == 0) {
        return NULL;
    }
    int v = 0;
    for (size_t i = 0; i < len; ++i) {
        int c = str[i];
        if (bor[v].next[c] == -1) {
            return NULL;
        }
        v = bor[v].next[c];
    }
    return &bor[v];
}

void
init_bor_and_stack() {
    bor = calloc(MAX_BOR_SIZE, sizeof(*bor));
    bor_size = 0, SP = 0;
    memset(bor[0].next, 255, sizeof bor[0].next);
    bor_size = 1;

    bor[0].table = new_table();
    for (int i = 0; i < N_SYMBOLS; i++) {
        bor[0].table.freq[i] = 1;
    }
}

/*================ INITIALIZE FUNCTIONS ================*/

static int
get_filesize(FILE *f) {
    fseek(f, 0, SEEK_END);
    int n_symbols = ftell(f);
    fseek(f, 0, SEEK_SET);
    return n_symbols;
}

static void
setup_buffers() {
    memset(buff_read, 0, sizeof(buff_read));
    memset(buff_write, 0, sizeof(buff_write));
    buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
    bits_to_follow = 0;
}

/*================ CONTEXT MODEL FUNCTIONS ================*/

void update_context(int context[CONTEXT_LEN], int c) {
    for (int i = 0; i < CONTEXT_LEN - 1; i++) {
        context[i] = context[i + 1];
    }
    context[CONTEXT_LEN - 1] = c;
}

void
update_model(int c) {
    while (SP) {
        SP--;
        if (stack[SP]->sum >= MAX_TOTAL_SUM)
            rescale(stack[SP]);
        stack[SP]->sum += 1;
        if (!stack[SP]->freq[c])
            stack[SP]->esc += 1;
        stack[SP]->freq[c] += 1;
    }
}

void
rescale(Table *table) {
    table->sum = 0;
    for (int i = 0; i < 256; i++) {
        table->freq[i] -= table->freq[i] >> 1;
        table->sum += table->freq[i];
    }
}

int
get_interval(Table *table, int c, interval_t *left, interval_t *right, interval_t *divider) {
    stack[SP++] = table;
    if (table->freq[c]) {
        interval_t CumFreqUnder = 0;
        for (int i = 0; i < c; i++)
            CumFreqUnder += table->freq[i];
        *left = CumFreqUnder, *right = *left + table->freq[c], *divider = table->sum + table->esc;
        return 1;
    } else {
        if (table->esc) {
            *left = table->sum, *right = *left + table->esc, *divider = table->sum + table->esc;
        }
        return 0;
    }
}

void
normalize_and_write(interval_t *p_left, interval_t *p_right) {
    interval_t left = *p_left, right = *p_right; // for don't torment with pointers
    for (;;) {
        if (right < HALF) {
            bits_plus_follow(0);
        } else if (left >= HALF) {
            bits_plus_follow(1);
            left -= HALF;
            right -= HALF;
        } else if ((left >= FIRST_QTR) && (right < THIRD_QTR)) {
            bits_to_follow++;
            left -= FIRST_QTR;
            right -= FIRST_QTR;
        } else
            break;
        left <<= 1;
        right <<= 1;
        right += 1;
    }
    *p_left = left, *p_right = right; // updating
}

void normalize_and_fill(interval_t *p_left, interval_t *p_right, interval_t val) {
    interval_t left = *p_left, right = *p_right;
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
    *p_left = left, *p_right = right; // updating
}

int encode_sym(Table *p_table, int c, interval_t *p_left_prev, interval_t *p_right_prev, interval_t *p_left,
               interval_t
               *p_right) {
    interval_t cum_freq_prev, cum_freq, divider;
    int success;
    success = get_interval(p_table, c, &cum_freq_prev, &cum_freq, &divider);

    *p_left = (*p_left_prev) + cum_freq_prev * ((*p_right_prev) - (*p_left_prev) + 1) / divider;
    *p_right = (*p_left_prev) + cum_freq * ((*p_right_prev) - (*p_left_prev) + 1) / divider - 1;

    normalize_and_write(p_left, p_right);

    *p_left_prev = *p_left, *p_right_prev = *p_right;
    return success;
}


