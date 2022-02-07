#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

enum {
    CT_LEN = 3,
    RESCALE = 20000,
    AGGR = 20,
};

/* Класс для организации ввода/вывода данных */
struct DFile {
    FILE *f;

    int ReadSymbol(void) {
        return getc(f);
    };

    int WriteSymbol(int c) {
        return putc(c, f);
    };

    FILE *GetFile(void) {
        return f;
    }

    void SetFile(FILE *file) {
        f = file;
    }
} DataFile, CompressedFile;

typedef unsigned int uint;
#define DO(n) for (int _ = 0; _ < n; _++)
#define TOP (1 << 24)
#define context(len) (&context[CT_LEN - len])

struct RangeCoder {
    uint code, range, FFNum, Cache;
    int64_t low; // Microsoft C/C++ 64-bit integer type
    FILE *f;

    void StartEncode(FILE *out) {
        low = FFNum = Cache = 0;
        range = (uint) -1;
        f = out;
    }

    void StartDecode(FILE *in) {
        code = 0;
        range = (uint) -1;
        f = in;
        DO(5) code = (code << 8) | getc(f);
    }

    void FinishEncode(void) {
        low += 1;
        DO (5) ShiftLow();
    }

    void encode(uint cumFreq, uint freq, uint totFreq) {
        low += cumFreq * (range /= totFreq);
        range *= freq;
        while (range < TOP) ShiftLow(), range <<= 8;
    }

    inline void ShiftLow(void) {
        if ((low >> 24) != 0xFF) {
            putc(Cache + (low >> 32), f);
            int c = 0xFF + (low >> 32);
            while (FFNum) putc(c, f), FFNum--;
            Cache = uint(low) >> 24;
        } else FFNum++;
        low = uint(low) << 8;
    }

    uint get_freq(uint totFreq) {
        return code / (range /= totFreq);
    }

    void decode_update(uint cumFreq, uint freq, uint totFreq) {
        code -= cumFreq * range;
        range *= freq;
        while (range < TOP) code = (code << 8) | getc(f), range <<= 8;
    }
} AC;

struct ContextModel {
    int esc, TotFr;
    int count[256];
};

typedef struct Vertex {
    int next[256];
    ContextModel *pl;
} Vertex;

typedef struct Bor {
    Vertex body[256 * 256 * 4];
    int sz;
} Bor;
Bor bor;

void init_bor() {
    Vertex *t = bor.body;
    memset(t[0].next, 255, sizeof t[0].next);
    bor.sz = 1;
}

Vertex *add_context(const int *str, int len) {
    int v = 0;
    Vertex *t = bor.body;
    for (size_t i = 0; i < len; ++i) {
        int c = str[i]; // в зависимости от алфавита
        if (t[v].next[c] == -1) {
            memset(t[bor.sz].next, 255, sizeof t[bor.sz].next);
            t[v].next[c] = bor.sz++;
        }
        v = t[v].next[c];
        t[v].pl = (ContextModel*) (calloc(1, sizeof(ContextModel)));
    }
    if (!t[v].pl) {
        printf("out of memory\n");
        exit(1);
    }
    return &t[v];
}

Vertex *get_vertex(const int *str, int len) {
    int v = 0;
    Vertex *t = bor.body;
    for (size_t i = 0; i < len; ++i) {
        int c = str[i]; // в зависимости от алфавита
        if (t[v].next[c] == -1) {
            return NULL;
        }
        v = t[v].next[c];
    }
    return &t[v];
}

ContextModel *stack[CT_LEN + 1];
int context[CT_LEN], SP;
unsigned char mask[32];

void init_model(void) {
    init_bor();
    bor.body[0].pl = (ContextModel*) (calloc(1, sizeof(ContextModel)));
    for (int j = 0; j < 256; j++)
        bor.body[0].pl->count[j] = 1;
    bor.body[0].pl->TotFr = 256;
    bor.body[0].pl->esc = 1;
    context[0] = SP = 0;
    memset(mask, 255, sizeof(mask));
}

int get_mask_bit(int n) {
    unsigned char n_byte = n / 8, n_bit = n % 8, bit = 1 << n_bit;
    return (mask[n_byte] & bit) >> n_bit;
}

void set_mask_bit(int n) {
    unsigned char n_byte = n / 8, n_bit = n % 8, bit = 1 << n_bit;
    mask[n_byte] &= ~bit;
}

int masked_val(ContextModel *CM, int c) {
    return CM->count[c] * get_mask_bit(c);
}

int masked_sum(ContextModel *CM) {
    int sum = 0;
    for (int i = 0; i < 256; i++) {
        sum += get_mask_bit(i) * CM->count[i];
    }
    return sum;
}

void set_mask(ContextModel *CM) {
    for (int i = 0; i < 256; i++)
        if (CM->count[i])
            set_mask_bit(i);
}

int encode_sym(ContextModel *CM, int c) {
    int TotFr_m = masked_sum(CM);
    if (CM->count[c]) {
        int CumFreqUnder = 0;
        int delta;
        for (int i = 0; i < c; i++) {
            delta = masked_val(CM, i);
            CumFreqUnder += delta;
        }
        AC.encode(CumFreqUnder, CM->count[c], TotFr_m + CM->esc);
        return 1;
    } else {
        if (CM->esc) {
            AC.encode(TotFr_m, CM->esc, TotFr_m + CM->esc);
        }
        set_mask(CM);
        return 0;
    }
}

int decode_sym(ContextModel *CM, int *c) {
    int TotFr_m = masked_sum(CM);
    if (!CM->esc) return 0;
    int cum_freq = AC.get_freq(TotFr_m + CM->esc);
    if (cum_freq < TotFr_m) {
        int CumFreqUnder = 0;
        int i = 0;
        for (;;) {
            if ((CumFreqUnder + masked_val(CM, i)) <= cum_freq)
                CumFreqUnder += masked_val(CM, i);
            else break;
            i++;
        }
        AC.decode_update(CumFreqUnder, CM->count[i], TotFr_m + CM->esc);
        *c = i;
        return 1;
    } else {
        AC.decode_update(TotFr_m, CM->esc, TotFr_m + CM->esc);
        set_mask(CM);
        return 0;
    }
}

void rescale(ContextModel *CM) {
    CM->TotFr = 0;
    for (int i = 0; i < 256; i++) {
        CM->count[i] -= CM->count[i] >> 1;
        CM->TotFr += CM->count[i];
    }
}

void update_model(int c) {
    while (SP) {
        SP--;
        if (stack[SP]->TotFr >= RESCALE)
            rescale(stack[SP]);
        stack[SP]->TotFr += AGGR;
        if (!stack[SP]->count[c])
            stack[SP]->esc += AGGR;
        stack[SP]->count[c] += AGGR;
    }
    memset(mask, 255, sizeof(mask));
}

void update_context(int c) {
    for (int i = 0; i < CT_LEN - 1; i++) {
        context[i] = context[i + 1];
    }
    context[CT_LEN - 1] = c;
}

int write_file_len (void) {
    FILE *ifp = DataFile.f, *ofp = CompressedFile.f;
    fseek(ifp, 0, SEEK_END);
    int n_symbols = ftell(ifp);
    fwrite(&n_symbols, sizeof(n_symbols), 1, ofp);
    fseek(ifp, 0, SEEK_SET);
    return n_symbols;
}

int read_file_len(void) {
    FILE *ifp = CompressedFile.f;
    int n_symbols;
    fread(&n_symbols, sizeof(n_symbols), 1, ifp);
    return n_symbols;
}

void write_to_end() {
    for (int len = CT_LEN; len >= 0; len--) {
        Vertex *v = get_vertex(context(len), len);
        ContextModel *cm;
        if (!v) {
            cm = add_context(context(len), len)->pl;
        } else {
            cm = v->pl;
        }
//        AC.encode(cm->TotFr, cm->esc, cm->TotFr + cm->esc);
        encode_sym(cm, 0);
    }
}

void encode(void) {
    int c, success;
    write_file_len();
    init_model();
    AC.StartEncode(CompressedFile.GetFile());
    while ((c = DataFile.ReadSymbol()) != EOF) {
        int len = CT_LEN;
        for (;;) {
            Vertex *v = get_vertex(context(len), len);
            ContextModel *cm;
            if (!v) {
                v = add_context(context(len), len);
                cm = v->pl;
            } else {
                cm = v->pl;
            }
            stack[SP++] = v->pl;
            success = encode_sym(cm, c);
            if (!success)
                len--;
            else
                break;
        }
        update_model(c);
        update_context(c);
    }
    write_to_end();
    AC.FinishEncode();
}

void decode(void) {
    int c, success;
    init_model();
    int file_len = read_file_len(), cur_len = 0;
    AC.StartDecode(CompressedFile.GetFile());
    for (;;) {
        int len = CT_LEN;
        for (;len >= 0 ;) {
            Vertex *v = get_vertex(context(len), len);
            ContextModel *cm;
            if (!v) {
                v = add_context(context(len), len);
                cm = v->pl;
            } else {
                cm = v->pl;
            }
            stack[SP++] = v->pl;
            success = decode_sym(cm, &c);
            if (!success)
                len--;
            else
                break;
        }
        if (len < 0) break;

        update_model(c);
        update_context(c);
        DataFile.WriteSymbol(c);
        cur_len++;
        if (cur_len == file_len) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *inf, *outf;

    char mode = 'c';
    char in[] = R"(C:\Users\1\Documents\EVM\compressor_8_cpp\my_tests\public_tests_programs\bcppcompiler.py)";
    char enc[] = R"(C:\Users\1\Documents\EVM\compressor_8_cpp\cmake-build-debug\2.txt)";
    char dec[] = R"(C:\Users\1\Documents\EVM\compressor_8_cpp\cmake-build-debug\3.txt)";

    if (mode == 'c') {
        inf = fopen(in, "rb");
        outf = fopen(enc, "wb");
        DataFile.SetFile(inf);
        CompressedFile.SetFile(outf);
        encode();
        fclose(inf);
        fclose(outf);
    } else if (mode == 'd') {
        inf = fopen(enc, "rb");
        outf = fopen(dec, "wb");
        CompressedFile.SetFile(inf);
        DataFile.SetFile(outf);
        decode();
        fclose(inf);
        fclose(outf);
    }
}