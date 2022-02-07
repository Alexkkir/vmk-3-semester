#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "ari.h"

#define sum_b(i) b[i + 1] + current_b[i + 1]
#define b(i) b[i + 1]
#define current_b(i) current_b[i + 1]
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
    ADD_TO_END = 50,
    BYTE_LEN = 8,
    BUF_LAST_IND = BUF_LEN,
    BUF_LAST_BIT = BUF_LEN * LEN_SYMBOL * BYTE_LEN,
    HASHTABLE_MAX = 1 << 18,
    STR_LEN = 3,
    TOO_RARE_USAGE = 10000,
};

static int THRESHOLD = 8192 * 8, AGGRESSIVENESS = 80;

#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>
#include <stddef.h>

/****************** DEFINTIIONS ******************/

#define HT_MINIMUM_CAPACITY 8
#define HT_LOAD_FACTOR 5
#define HT_MINIMUM_THRESHOLD (HT_MINIMUM_CAPACITY) * (HT_LOAD_FACTOR)

#define HT_GROWTH_FACTOR 2
#define HT_SHRINK_THRESHOLD (1 / 4)

#define HT_ERROR -1
#define HT_SUCCESS 0

#define HT_UPDATED 1
#define HT_INSERTED 0

#define HT_NOT_FOUND 0
#define HT_FOUND 01

#define HT_INITIALIZER {0, 0, 0, 0, 0, NULL, NULL, NULL};

typedef int (*comparison_t)(void *, void *, size_t);

typedef size_t (*hash_t)(void *, size_t);

/****************** STRUCTURES ******************/

typedef struct HTNode {
    struct HTNode *next;
    void *key;
    void *value;

} HTNode;

typedef struct HashTable {
    size_t size;
    size_t threshold;
    size_t capacity;

    size_t key_size;
    size_t value_size;

    comparison_t compare;
    hash_t hash;

    HTNode **nodes;

} HashTable;

/****************** INTERFACE ******************/

/* Setup */
int ht_setup(HashTable *table,
             size_t key_size,
             size_t value_size,
             size_t capacity);

int ht_copy(HashTable *first, HashTable *second);

int ht_move(HashTable *first, HashTable *second);

int ht_swap(HashTable *first, HashTable *second);

/* Destructor */
int ht_destroy(HashTable *table);

int ht_insert(HashTable *table, void *key, void *value);

int ht_contains(HashTable *table, void *key);

void *ht_lookup(HashTable *table, void *key);

const void *ht_const_lookup(const HashTable *table, void *key);

#define HT_LOOKUP_AS(type, table_pointer, key_pointer) \
    (*(type*)ht_lookup((table_pointer), (key_pointer)))

int ht_erase(HashTable *table, void *key);

int ht_clear(HashTable *table);

int ht_is_empty(HashTable *table);

bool ht_is_initialized(HashTable *table);

int ht_reserve(HashTable *table, size_t minimum_capacity);

/****************** PRIVATE ******************/

void _ht_int_swap(size_t *first, size_t *second);

void _ht_pointer_swap(void **first, void **second);

size_t _ht_default_hash(void *key, size_t key_size);

int _ht_default_compare(void *first_key, void *second_key, size_t key_size);

size_t _ht_hash(const HashTable *table, void *key);

bool _ht_equal(const HashTable *table, void *first_key, void *second_key);

bool _ht_should_grow(HashTable *table);

bool _ht_should_shrink(HashTable *table);

HTNode *_ht_create_node(HashTable *table, void *key, void *value, HTNode *next);

int _ht_push_front(HashTable *table, size_t index, void *key, void *value);

void _ht_destroy_node(HTNode *node);

int _ht_adjust_capacity(HashTable *table);

int _ht_allocate(HashTable *table, size_t capacity);

int _ht_resize(HashTable *table, size_t new_capacity);

void _ht_rehash(HashTable *table, HTNode **old, size_t old_capacity);

#endif /* HASHTABLE_H */

static unsigned char buff_write[BUF_LEN] = {0}, buff_read[BUF_LEN] = {0};
static int buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
static int bits_to_follow = 0;
static FILE *ifp, *ofp;

typedef struct {
    interval_t *b;
    int last_access;
} ht_val;

static void write_bit(int bit);

static void bits_plus_follow(int bit);

static int get_bit();

static int get_symbol();

static void write_symbol(int symbol);

void compress_ppm(char *ifile, char *ofile) {
    ifp = (FILE *) fopen(ifile, "rb");
    ofp = (FILE *) fopen(ofile, "wb");

    HashTable ht;
    ht_setup(&ht, sizeof(char *[STR_LEN]), sizeof(ht_val), 10);
    ht_reserve(&ht, HASHTABLE_MAX);

    memset(buff_read, 0, sizeof(buff_read));
    memset(buff_write, 0, sizeof(buff_write));
    buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
    bits_to_follow = 0;

    // definition of aux structures
    interval_t b[N_SYMBOLS + 1];
    for (int i = 0; i <= N_SYMBOLS; i++) {
        b[i] = i;
    }

    fseek(ifp, 0, SEEK_END);
    int n_symbols = ftell(ifp);
    fwrite(&n_symbols, sizeof(n_symbols), 1, ofp);
    fseek(ifp, 0, SEEK_SET);

    interval_t li = 0, hi = 0, li_prev = 0, hi_prev = INTERVAL_LEN, divider;
    int add_to_end = ADD_TO_END, max_freq = 1, max_freq_ind = 0;

    int c; // depends on constants
    int index;
    int n_iter;

    unsigned char last_symbols[STR_LEN] = {0};
    interval_t *current_b;

    for (n_iter = 0;; n_iter++) {
//        printf("%d\n", n_iter);
        c = get_symbol(); // Читаем символ
        if (c == -1 || n_iter >= n_symbols) {
            if (add_to_end > 0) {
                index = max_freq_ind;
                add_to_end--;
            } else
                break;
        } else {
            index = c;
        }

        // Занимаемся дополнительными таблицами
        if (ht_contains(&ht, &last_symbols)) {
            ht_val *p_elem = ht_lookup(&ht, &last_symbols);
            p_elem->last_access = n_iter;
            current_b = p_elem->b;
        } else {
            ht_val val;
            val.b = calloc(N_SYMBOLS + 1, sizeof(interval_t));
            for (int i = 0; i < N_SYMBOLS + 1; i++) {
                val.b[i] = i;
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
            current_b(i) += AGGRESSIVENESS;
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
            max_freq = b(max_freq_ind) - b(max_freq_ind - 1);
        }
        if (current_b(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = current_b(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = current_b(i);
                current_b(i) -= total_delta;
            }
        }

        // обновление максимальной частоты
        if (b(index) - b(index - 1) > max_freq) {
            max_freq = b(index) - b(index - 1);
            max_freq_ind = index;
        }

        // обновляем контекст
        for (int i = 0; i < STR_LEN - 1; i++) {
            last_symbols[i] = last_symbols[i + 1];
        }
        last_symbols[STR_LEN - 1] = c;
    }
    // cleaning of the buffer
    fwrite(buff_write, 1, (buff_write_pos + BYTE_LEN - 1) / BYTE_LEN, ofp);
    fclose(ifp);
    fclose(ofp);
}

void decompress_ppm(char *ifile, char *ofile) {
    ifp = (FILE *) fopen(ifile, "rb");
    ofp = (FILE *) fopen(ofile, "wb");

    HashTable ht;
    ht_setup(&ht, sizeof(char *[STR_LEN]), sizeof(ht_val), 10);
    ht_reserve(&ht, HASHTABLE_MAX);

    memset(buff_read, 0, sizeof(buff_read));
    memset(buff_write, 0, sizeof(buff_write));
    buff_write_pos = 0, buff_read_ind = BUF_LAST_IND, buff_read_pos = BUF_LAST_BIT, buff_write_ind = 0;
    bits_to_follow = 0;

    // definition of aux structures
    interval_t b[N_SYMBOLS + 1];
    for (int i = 0; i <= N_SYMBOLS; i++) {
        b[i] = i;
    }

    int n_symbols;
    fread(&n_symbols, sizeof(n_symbols), 1, ifp);
    interval_t val;
    unsigned char buff_help[sizeof(val)] = {0};
    fread(buff_help, 1, 4, ifp);
    unsigned char buff_inv[sizeof(val)] = {buff_help[3], buff_help[2], buff_help[1], buff_help[0], 0, 0, 0, 0};
    val = *((interval_t *) buff_inv);

    interval_t li = 0, hi = 0, li_prev = 0, hi_prev = INTERVAL_LEN, divider = b[N_SYMBOLS];

    unsigned int c; // depends from constants
    int index;

    unsigned char last_symbols[STR_LEN];
    interval_t *current_b = {0};

    for (int n_iter = 0; n_iter < n_symbols; n_iter++) {
//        printf("%d\n", n_iter);
        // ищем индекс очередного символа
        for (index = 0; index < N_SYMBOLS; index++) {
            interval_t addon_ind, addon_div;
            if (ht_contains(&ht, last_symbols)) {
                ht_val *elem;
                elem = ht_lookup(&ht, last_symbols);
                addon_ind = elem->b(index);
                addon_div = elem->b(N_SYMBOLS - 1);
            } else {
                addon_ind = index;
                addon_div = N_SYMBOLS;
            }

            divider = b(N_SYMBOLS - 1) + addon_div;
            hi = li_prev + (b(index) + addon_ind) * (hi_prev - li_prev + 1) / divider - 1;
            if (val <= hi) break;
        }
        write_symbol(index);

        // Занимаемся дополнительными таблицами
        if (ht_contains(&ht, &last_symbols)) {
            ht_val *p_elem = ht_lookup(&ht, &last_symbols);
            p_elem->last_access = n_iter;
            current_b = p_elem->b;
        } else {
            ht_val val;
            val.b = calloc(N_SYMBOLS + 1, sizeof(interval_t));
            for (int i = 0; i < N_SYMBOLS + 1; i++) {
                b[i] = i;
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
            current_b(i) += AGGRESSIVENESS;
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
        if (current_b(N_SYMBOLS - 1) >= THRESHOLD) {
            for (int i = 0; i < N_SYMBOLS; i++) {
                table_i = current_b(i) - old_b_i_prev;
                total_delta += ((table_i + 1) >> 1) - 1;
                old_b_i_prev = current_b(i);
                current_b(i) -= total_delta;
            }
        }

        // обновляем конекст
        for (int i = 0; i < STR_LEN - 1; i++) {
            last_symbols[i] = last_symbols[i + 1];
        }
        last_symbols[STR_LEN - 1] = c;
    }

    // cleaning of the buffer
    fwrite(buff_write, 1, buff_write_ind, ofp);
    fclose(ifp);
    fclose(ofp);
}


void bits_plus_follow(int bit) {
    write_bit(bit);
    for (; bits_to_follow > 0; bits_to_follow--)
        write_bit(!bit);
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

int ht_setup(HashTable *table,
             size_t key_size,
             size_t value_size,
             size_t capacity) {
    assert(table != NULL);

    if (table == NULL) return HT_ERROR;

    if (capacity < HT_MINIMUM_CAPACITY) {
        capacity = HT_MINIMUM_CAPACITY;
    }

    if (_ht_allocate(table, capacity) == HT_ERROR) {
        return HT_ERROR;
    }

    table->key_size = key_size;
    table->value_size = value_size;
    table->hash = _ht_default_hash;
    table->compare = _ht_default_compare;
    table->size = 0;

    return HT_SUCCESS;
}

int ht_copy(HashTable *first, HashTable *second) {
    size_t chain;
    HTNode *node;

    assert(first != NULL);
    assert(ht_is_initialized(second));

    if (first == NULL) return HT_ERROR;
    if (!ht_is_initialized(second)) return HT_ERROR;

    if (_ht_allocate(first, second->capacity) == HT_ERROR) {
        return HT_ERROR;
    }

    first->key_size = second->key_size;
    first->value_size = second->value_size;
    first->hash = second->hash;
    first->compare = second->compare;
    first->size = second->size;

    for (chain = 0; chain < second->capacity; ++chain) {
        for (node = second->nodes[chain]; node; node = node->next) {
            if (_ht_push_front(first, chain, node->key, node->value) == HT_ERROR) {
                return HT_ERROR;
            }
        }
    }

    return HT_SUCCESS;
}

int ht_move(HashTable *first, HashTable *second) {
    assert(first != NULL);
    assert(ht_is_initialized(second));

    if (first == NULL) return HT_ERROR;
    if (!ht_is_initialized(second)) return HT_ERROR;

    *first = *second;
    second->nodes = NULL;

    return HT_SUCCESS;
}

int ht_swap(HashTable *first, HashTable *second) {
    assert(ht_is_initialized(first));
    assert(ht_is_initialized(second));

    if (!ht_is_initialized(first)) return HT_ERROR;
    if (!ht_is_initialized(second)) return HT_ERROR;

    _ht_int_swap(&first->key_size, &second->key_size);
    _ht_int_swap(&first->value_size, &second->value_size);
    _ht_int_swap(&first->size, &second->size);
    _ht_pointer_swap((void **) &first->hash, (void **) &second->hash);
    _ht_pointer_swap((void **) &first->compare, (void **) &second->compare);
    _ht_pointer_swap((void **) &first->nodes, (void **) &second->nodes);

    return HT_SUCCESS;
}

int ht_destroy(HashTable *table) {
    HTNode *node;
    HTNode *next;
    size_t chain;

    assert(ht_is_initialized(table));
    if (!ht_is_initialized(table)) return HT_ERROR;

    for (chain = 0; chain < table->capacity; ++chain) {
        node = table->nodes[chain];
        while (node) {
            next = node->next;
            _ht_destroy_node(node);
            node = next;
        }
    }

    free(table->nodes);

    return HT_SUCCESS;
}

int ht_insert(HashTable *table, void *key, void *value) {
    size_t index;
    HTNode *node;

    assert(ht_is_initialized(table));
    assert(key != NULL);

    if (!ht_is_initialized(table)) return HT_ERROR;
    if (key == NULL) return HT_ERROR;

    if (_ht_should_grow(table)) {
        _ht_adjust_capacity(table);
    }

    index = _ht_hash(table, key);
    for (node = table->nodes[index]; node; node = node->next) {
        if (_ht_equal(table, key, node->key)) {
            memcpy(node->value, value, table->value_size);
            return HT_UPDATED;
        }
    }

    if (_ht_push_front(table, index, key, value) == HT_ERROR) {
        return HT_ERROR;
    }

    ++table->size;

    return HT_INSERTED;
}

int ht_contains(HashTable *table, void *key) {
    size_t index;
    HTNode *node;

    assert(ht_is_initialized(table));
    assert(key != NULL);

    if (!ht_is_initialized(table)) return HT_ERROR;
    if (key == NULL) return HT_ERROR;

    index = _ht_hash(table, key);
    for (node = table->nodes[index]; node; node = node->next) {
        if (_ht_equal(table, key, node->key)) {
            return HT_FOUND;
        }
    }

    return HT_NOT_FOUND;
}

void *ht_lookup(HashTable *table, void *key) {
    HTNode *node;
    size_t index;

    assert(table != NULL);
    assert(key != NULL);

    if (table == NULL) return NULL;
    if (key == NULL) return NULL;

    index = _ht_hash(table, key);
    for (node = table->nodes[index]; node; node = node->next) {
        if (_ht_equal(table, key, node->key)) {
            return node->value;
        }
    }

    return NULL;
}

const void *ht_const_lookup(const HashTable *table, void *key) {
    const HTNode *node;
    size_t index;

    assert(table != NULL);
    assert(key != NULL);

    if (table == NULL) return NULL;
    if (key == NULL) return NULL;

    index = _ht_hash(table, key);
    for (node = table->nodes[index]; node; node = node->next) {
        if (_ht_equal(table, key, node->key)) {
            return node->value;
        }
    }

    return NULL;
}

int ht_erase(HashTable *table, void *key) {
    HTNode *node;
    HTNode *previous;
    size_t index;

    assert(table != NULL);
    assert(key != NULL);

    if (table == NULL) return HT_ERROR;
    if (key == NULL) return HT_ERROR;

    index = _ht_hash(table, key);
    node = table->nodes[index];

    for (previous = NULL; node; previous = node, node = node->next) {
        if (_ht_equal(table, key, node->key)) {
            if (previous) {
                previous->next = node->next;
            } else {
                table->nodes[index] = node->next;
            }

            _ht_destroy_node(node);
            --table->size;

            if (_ht_should_shrink(table)) {
                if (_ht_adjust_capacity(table) == HT_ERROR) {
                    return HT_ERROR;
                }
            }

            return HT_SUCCESS;
        }
    }

    return HT_NOT_FOUND;
}

int ht_clear(HashTable *table) {
    assert(table != NULL);
    assert(table->nodes != NULL);

    if (table == NULL) return HT_ERROR;
    if (table->nodes == NULL) return HT_ERROR;

    ht_destroy(table);
    _ht_allocate(table, HT_MINIMUM_CAPACITY);
    table->size = 0;

    return HT_SUCCESS;
}

int ht_is_empty(HashTable *table) {
    assert(table != NULL);
    if (table == NULL) return HT_ERROR;
    return table->size == 0;
}

bool ht_is_initialized(HashTable *table) {
    return table != NULL && table->nodes != NULL;
}

int ht_reserve(HashTable *table, size_t minimum_capacity) {
    assert(ht_is_initialized(table));
    if (!ht_is_initialized(table)) return HT_ERROR;

    /*
     * We expect the "minimum capacity" to be in elements, not in array indices.
     * This encapsulates the design.
     */
    if (minimum_capacity > table->threshold) {
        return _ht_resize(table, minimum_capacity / HT_LOAD_FACTOR);
    }

    return HT_SUCCESS;
}

/****************** PRIVATE ******************/

void _ht_int_swap(size_t *first, size_t *second) {
    size_t temp = *first;
    *first = *second;
    *second = temp;
}

void _ht_pointer_swap(void **first, void **second) {
    void *temp = *first;
    *first = *second;
    *second = temp;
}

int _ht_default_compare(void *first_key, void *second_key, size_t key_size) {
    return memcmp(first_key, second_key, key_size);
}

size_t _ht_default_hash(void *raw_key, size_t key_size) {
    // djb2 string hashing algorithm
    // sstp://www.cse.yorku.ca/~oz/hash.ssml
    size_t byte;
    size_t hash = 5381;
    char *key = raw_key;

    for (byte = 0; byte < key_size; ++byte) {
        // (hash << 5) + hash = hash * 33
        hash = ((hash << 5) + hash) ^ key[byte];
    }

    return hash;
}

size_t _ht_hash(const HashTable *table, void *key) {
#ifdef HT_USING_POWER_OF_TWO
    return table->hash(key, table->key_size) & table->capacity;
#else
    return table->hash(key, table->key_size) % table->capacity;
#endif
}

bool _ht_equal(const HashTable *table, void *first_key, void *second_key) {
    return table->compare(first_key, second_key, table->key_size) == 0;
}

bool _ht_should_grow(HashTable *table) {
    assert(table->size <= table->capacity);
    return table->size == table->capacity;
}

bool _ht_should_shrink(HashTable *table) {
    assert(table->size <= table->capacity);
    return table->size == table->capacity * HT_SHRINK_THRESHOLD;
}

HTNode *
_ht_create_node(HashTable *table, void *key, void *value, HTNode *next) {
    HTNode *node;

    assert(table != NULL);
    assert(key != NULL);
    assert(value != NULL);

    if ((node = malloc(sizeof *node)) == NULL) {
        return NULL;
    }
    if ((node->key = malloc(table->key_size)) == NULL) {
        return NULL;
    }
    if ((node->value = malloc(table->value_size)) == NULL) {
        return NULL;
    }

    memcpy(node->key, key, table->key_size);
    memcpy(node->value, value, table->value_size);
    node->next = next;

    return node;
}

int _ht_push_front(HashTable *table, size_t index, void *key, void *value) {
    table->nodes[index] = _ht_create_node(table, key, value, table->nodes[index]);
    return table->nodes[index] == NULL ? HT_ERROR : HT_SUCCESS;
}

void _ht_destroy_node(HTNode *node) {
    assert(node != NULL);

    free(node->key);
    free(node->value);
    free(node);
}

int _ht_adjust_capacity(HashTable *table) {
    return _ht_resize(table, table->size * HT_GROWTH_FACTOR);
}

int _ht_allocate(HashTable *table, size_t capacity) {
    if ((table->nodes = malloc(capacity * sizeof(HTNode *))) == NULL) {
        return HT_ERROR;
    }
    memset(table->nodes, 0, capacity * sizeof(HTNode *));

    table->capacity = capacity;
    table->threshold = capacity * HT_LOAD_FACTOR;

    return HT_SUCCESS;
}

int _ht_resize(HashTable *table, size_t new_capacity) {
    HTNode **old;
    size_t old_capacity;

    if (new_capacity < HT_MINIMUM_CAPACITY) {
        if (table->capacity > HT_MINIMUM_CAPACITY) {
            new_capacity = HT_MINIMUM_CAPACITY;
        } else {
            /* NO-OP */
            return HT_SUCCESS;
        }
    }

    old = table->nodes;
    old_capacity = table->capacity;
    if (_ht_allocate(table, new_capacity) == HT_ERROR) {
        return HT_ERROR;
    }

    _ht_rehash(table, old, old_capacity);

    free(old);

    return HT_SUCCESS;
}

void _ht_rehash(HashTable *table, HTNode **old, size_t old_capacity) {
    HTNode *node;
    HTNode *next;
    size_t new_index;
    size_t chain;

    for (chain = 0; chain < old_capacity; ++chain) {
        for (node = old[chain]; node;) {
            next = node->next;

            new_index = _ht_hash(table, node->key);
            node->next = table->nodes[new_index];
            table->nodes[new_index] = node;

            node = next;
        }
    }
}