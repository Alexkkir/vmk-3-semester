#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <stdlib.h>

enum {
    CT_LEN = 3,
    RESCALE = 20000,
    AGGR = 20,
    BODY = 256 * 256 * 64,
};

/* Класс для организации ввода/вывода данных */
typedef struct DFile {
    FILE *f;
} DFile; DFile DataFile, CompressedFile;

int ReadSymbol(DFile *obj) {
    return getc(obj->f);
};

int WriteSymbol(DFile *obj, int c) {
    return putc(c, obj->f);
};

FILE *GetFile(DFile *obj) {
    return obj->f;
}

void SetFile(DFile *obj, FILE *file) {
    obj->f = file;
}

typedef unsigned int uint;
#define DO(n) for (int _ = 0; _ < n; _++)
#define TOP (1 << 24)
#define context(len) (&context[CT_LEN - len])

typedef struct RangeCoder {
    uint code, range, FFNum, Cache;
    int64_t low; // Microsoft C/C++ 64-bit integer type
    FILE *f;
} RangeCoder; struct RangeCoder AC;

void StartEncode(RangeCoder *obj, FILE *out) {
    obj->low = obj->FFNum = obj->Cache = 0;
    obj->range = (uint) -1;
    obj->f = out;
}

void StartDecode(RangeCoder *obj, FILE *in) {
    obj->code = 0;
    obj->range = (uint) -1;
    obj->f = in;
    DO(5) obj->code = (obj->code << 8) | getc(obj->f);
}

void ShiftLow(RangeCoder *obj);

void FinishEncode(RangeCoder *obj) {
    obj->low += 1;
    DO (5) ShiftLow(obj);
}

void encode(RangeCoder *obj, uint cumFreq, uint freq, uint totFreq) {
    obj->low += cumFreq * (obj->range /= totFreq);
    obj->range *= freq;
    while (obj->range < TOP) ShiftLow(obj), obj->range <<= 8;
}

void ShiftLow(RangeCoder *obj) {
    if ((obj->low >> 24) != 0xFF) {
        putc(obj->Cache + (obj->low >> 32), obj->f);
        int c = 0xFF + (obj->low >> 32);
        while (obj->FFNum) putc(c, obj->f), obj->FFNum--;
        obj->Cache = ((uint)obj->low) >> 24;
    } else obj->FFNum++;
    obj->low = ((uint)obj->low) << 8;
}

uint get_freq(RangeCoder *obj, uint totFreq) {
    return obj->code / (obj->range /= totFreq);
}

void decode_update(RangeCoder *obj, uint cumFreq, uint freq, uint totFreq) {
    obj->code -= cumFreq * obj->range;
    obj->range *= freq;
    while (obj->range < TOP) obj->code = (obj->code << 8) | getc(obj->f), obj->range <<= 8;
}

typedef struct ContextModel {
    int esc, TotFr;
    int count[256];
} ContextModel;

typedef struct Vertex {
    int next[256];
    ContextModel *pl;
} Vertex;

typedef struct Bor {
    Vertex *body;
    int sz;
} Bor;
Bor bor;

void init_bor() {
    bor.body = calloc(BODY, sizeof(Vertex));
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
        encode(&AC, CumFreqUnder, CM->count[c], TotFr_m + CM->esc);
        return 1;
    } else {
        if (CM->esc) {
            encode(&AC, TotFr_m, CM->esc, TotFr_m + CM->esc);
        }
        set_mask(CM);
        return 0;
    }
}

int decode_sym(ContextModel *CM, int *c) {
    int TotFr_m = masked_sum(CM);
    if (!CM->esc) return 0;
    int cum_freq = get_freq(&AC, TotFr_m + CM->esc);
    if (cum_freq < TotFr_m) {
        int CumFreqUnder = 0;
        int i = 0;
        for (;;) {
            if ((CumFreqUnder + masked_val(CM, i)) <= cum_freq)
                CumFreqUnder += masked_val(CM, i);
            else break;
            i++;
        }
        decode_update(&AC, CumFreqUnder, CM->count[i], TotFr_m + CM->esc);
        *c = i;
        return 1;
    } else {
        decode_update(&AC, TotFr_m, CM->esc, TotFr_m + CM->esc);
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

void compress_ppm(char *ifile, char *ofile) {
    FILE *inf, *outf;
    inf = fopen(ifile, "rb");
    outf = fopen(ofile, "wb");
    SetFile(&DataFile, inf);
    SetFile(&CompressedFile, outf);

    int c, success;
    write_file_len();
    init_model();
    StartEncode(&AC, GetFile(&CompressedFile));

    while ((c = ReadSymbol(&DataFile)) != EOF) {
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
    FinishEncode(&AC);

    fclose(inf);
    fclose(outf);
}

void decompress_ppm(char *ifile, char *ofile) {
    FILE *inf, *outf;
    inf = fopen(ifile, "rb");
    outf = fopen(ofile, "wb");
    SetFile(&CompressedFile, inf);
    SetFile(&DataFile, outf);

    int c, success;
    init_model();
    int file_len = read_file_len(), cur_len = 0;
    if (file_len == 0) {
        fclose(inf);
        fclose(outf);
        return;
    }
    StartDecode(&AC, GetFile(&CompressedFile));
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
        WriteSymbol(&DataFile, c);
        cur_len++;
        if (cur_len == file_len) {
            break;
        }
    }

    fclose(inf);
    fclose(outf);
}