#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ari.h"

struct freq_sym {
    int freq;
    unsigned char sym;
};

void bits_plus_follow(FILE *ofp, unsigned char *p_buff, int *p_pos, int bit, int *p_bits_to_follow);
int read_bit(FILE *ifp, unsigned char *p_buff, int *p_pos);
void write_bit(FILE *ofp, unsigned char *p_buff, int *p_pos, int bit);
int comp(const struct freq_sym *i, const struct freq_sym *j);

enum {
    LEN_SYMBOL = 1, N_SYMBOLS = 256
};

void compress_ari(char *ifile, char *ofile) {
    FILE *ifp = (FILE *) fopen(ifile, "rb");
    FILE *ofp = (FILE *) fopen(ofile, "wb");

    struct freq_sym table[N_SYMBOLS];
    memset(table, 0, sizeof(table));
    for (int i = 0; i < N_SYMBOLS; i++) {
        table[i].sym = i;
    }

    unsigned char buff[LEN_SYMBOL];
    int n, n_symbols = 0;
    while (1) {
        n = fread(buff, 1, sizeof(buff), ifp);
        if (n == 0) {
            break;
        }
        int index = buff[0];
        table[index].freq++;
        n_symbols++;
    }
    qsort(table, N_SYMBOLS, sizeof(table[0]), (int (*)(const void *, const void *)) comp);
    int new_order[N_SYMBOLS];
    for (int i = 0; i < N_SYMBOLS; i++) {
        new_order[table[i].sym] = i;
    }

    int b[N_SYMBOLS + 1];
    memset(b, 0, sizeof(b));
    for (int i = 1; i < N_SYMBOLS; i++) {
        b[i] = b[i - 1] + table[i - 1].freq;
    }

    fwrite(&n_symbols, 1, sizeof(n_symbols), ofp);
    fwrite(new_order, 1, sizeof(new_order), ofp);
    for (int i = 0; i < N_SYMBOLS; i++) {
        fwrite(&table[i].freq, 1, sizeof(table[i].freq), ofp);
    }
    fseek(ifp, 0, SEEK_SET);

    int l0 = 0, h0 = 65535, delitel = b[N_SYMBOLS - 1]; // = 10
    int First_qtr = (h0 + 1) / 4, Half = First_qtr * 2, Third_qtr = First_qtr * 3;
    int bits_to_follow = 0; // = 49152, Сколько бит сбрасывать
    unsigned char c;
    int li, li_1, hi, hi_1;
    li_1 = l0, hi_1 = h0;

    unsigned char buff_wr = 0;
    int buff_pos = 0;

    int n_empty_symbols = 10;
    for (int iter = 0; ; iter++) {
        n = fread(buff, 1, sizeof buff, ifp); // Читаем символ
        if (n == 0) {
            if (n_empty_symbols > 0) {
                c = table[0].sym;
                n_empty_symbols--;
            } else {
                break;
            }
        } else {
            c = buff[0];
        }
        int j = c;
        li = li_1 + b[new_order[j] - 1 + 1] * (hi_1 - li_1 + 1) / delitel;
        hi = li_1 + b[new_order[j] + 1] * (hi_1 - li_1 + 1) / delitel - 1;
        for (;;) {       // Обрабатываем варианты
            if (hi < Half) // переполнения
                bits_plus_follow(ofp, &buff_wr, &buff_pos, 0, &bits_to_follow);
            else if (li >= Half) {
                bits_plus_follow(ofp, &buff_wr, &buff_pos, 1, &bits_to_follow);
                li -= Half;
                hi -= Half;
            } else if ((li >= First_qtr) && (hi < Third_qtr)) {
                bits_to_follow++;
                li -= First_qtr;
                hi -= First_qtr;
            } else
                break;
            li += li;
            hi += hi + 1;
        }
        li_1 = li, hi_1 = hi;
    }

    for (int i = 0; i < 16; i++) { // Дописываем: 1) остаток из буффера вывода 2) записать дополнительные нули
        write_bit(ofp, &buff_wr, &buff_pos, 0);
    }
    fclose(ifp);
    fclose(ofp);
}

int comp(const struct freq_sym *i, const struct freq_sym *j) {
    return -(i->freq - j->freq);
}

void write_bit(FILE *ofp, unsigned char *p_buff, int *p_pos, int bit) {
    unsigned char b = 1 << (sizeof(*p_buff) * 8 - *p_pos - 1);
    *p_buff = (*p_buff) | (b * bit);
    (*p_pos)++;
    if (*p_pos == sizeof(*p_buff) * 8) {
        fwrite(p_buff, 1, sizeof(*p_buff), ofp);
//        printf("Written: %x\n", *p_buff);
        *p_buff = 0;
        *p_pos = 0;
    }
}


void decompress_ari(char *ifile, char *ofile) {
    FILE *ifp = (FILE *) fopen(ifile, "rb");
    FILE *ofp = (FILE *) fopen(ofile, "wb");

    /** PUT YOUR CODE HERE
     * implement an arithmetic encoding algorithm for decompression
     * don't forget to change header file `ari.h`
     */

    int n_symbols;
    fread(&n_symbols, 1, sizeof(n_symbols), ifp);

    int new_order[N_SYMBOLS];
    for (int i = 0; i < N_SYMBOLS; i++) {
        fread(&new_order[i], 1, sizeof(new_order[0]), ifp);
    }

    int table[N_SYMBOLS];
    for (int i = 0; i < N_SYMBOLS; i++) {
        fread(&table[i], 1, sizeof(table[0]), ifp);
    }


    int b[N_SYMBOLS + 1];
    memset(b, 0, sizeof(b));
    for (int i = 1; i < N_SYMBOLS; i++) {
        b[i] = b[i - 1] + table[i - 1];
    }

    int l0 = 0, h0 = 65535, delitel = b[N_SYMBOLS - 1]; // = 10
    int First_qtr = (h0 + 1) / 4, Half = First_qtr * 2, Third_qtr = First_qtr * 3;
    int bits_to_follow = 0; // = 49152, Сколько бит сбрасывать
    unsigned char c;
    int li, li_1, hi, hi_1;
    li_1 = l0, hi_1 = h0;

    unsigned char buff[4] = {0};
    unsigned char buff_rd = 0;
    int buff_pos = 0;
    int value = 0;
    fread(&buff, 1, 2, ifp);
    unsigned char swap_buff[4] = {buff[1], buff[0], 0, 0};
    value = *((int*) swap_buff);

    for (int iter = 0; iter < n_symbols; iter++) {
        int j;
        for (j = 0; j < N_SYMBOLS; j++) {
            li = li_1 + b[j - 1 + 1] * (hi_1 - li_1 + 1) / delitel;
            hi = li_1 + b[j + 1] * (hi_1 - li_1 + 1) / delitel - 1;
            if (li <= value && value < hi) break;
        }

        for (int i = 0; i < N_SYMBOLS; i++) {
            if (new_order[i] == j) {
                c = i;
                break;
            }
        }
        int xxx = 1;
        for (;;) {       // Обрабатываем варианты
            if (hi < Half) // переполнения
                ;
            else if (li >= Half) {
                value -= Half;
                li -= Half;
                hi -= Half;
            } else if ((li >= First_qtr) && (hi < Third_qtr)) {
                value -= First_qtr;
                li -= First_qtr;
                hi -= First_qtr;
            } else
                break;
            li += li;
            hi += hi + 1;
            value += value + read_bit(ifp, &buff_rd, &buff_pos);
        }
        fwrite(&c, 1, sizeof(c), ofp);
        li_1 = li, hi_1 = hi;
        xxx = 1;
    }

    fclose(ifp);
    fclose(ofp);
}

int read_bit(FILE *ifp, unsigned char *p_buff, int *p_pos) {
    if (*p_pos == 0 || *p_pos == sizeof(*p_buff) * 8) {
        *p_buff = 0;
        *p_pos = 0;
        if (!feof(ifp))
            fread(p_buff, 1, sizeof(*p_buff), ifp);
    }
    unsigned char b = 1 << (sizeof(*p_buff) * 8 - (*p_pos) - 1);
    (*p_pos)++;
    int ans = ((*p_buff) & b) >> (sizeof(*p_buff) * 8 - (*p_pos));
    return ans;
}

void bits_plus_follow(FILE *ofp, unsigned char *p_buff, int *p_pos, int bit, int *p_bits_to_follow) {
    write_bit(ofp, p_buff, p_pos, bit);
    for (; *p_bits_to_follow > 0; (*p_bits_to_follow)--)
        write_bit(ofp, p_buff, p_pos, !bit);
}
