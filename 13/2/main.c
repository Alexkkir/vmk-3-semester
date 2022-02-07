#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <assert.h>
#include <limits.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

enum { RADIX = 10, MAX_INT64_BIAS = 70, BYTE = 8 };

long str_to_int(char *str) {
    return strtol(str, NULL, RADIX);
}

int64_t calc_max_size(int64_t block_size, int64_t block_num_size, int64_t inode_direct_block_count) {
    int overflow_1 = 0, overflow_2 = 0;
    int64_t val_1 = 0, val_2 = 0, tmp;

    // val_1 = 2 ^ (8 * block_num_size) * block_size
    overflow_1 |= __builtin_mul_overflow(BYTE, block_num_size, &tmp);
    if (0 <= tmp && tmp <= MAX_INT64_BIAS) {
        val_1 = 1;
        for (int i = 0, n_it = tmp; i < n_it; i++) {
            overflow_1 |= __builtin_mul_overflow(2, val_1, &tmp);
            val_1 = tmp;
        }
        overflow_1 |= __builtin_mul_overflow(block_size, val_1, &tmp);
        val_1 = tmp;
    } else {
        overflow_1 = 1;
    }

    // val_2 = (inode_direct_block_count + x + x*x + x*x*x) * block_size
    int64_t x, x_2, x_3;
    x = block_size / block_num_size;
    overflow_2 |= __builtin_mul_overflow(x, x, &x_2);
    overflow_2 |= __builtin_mul_overflow(x, x_2, &x_3);
    val_2 = inode_direct_block_count;
    overflow_2 |= __builtin_add_overflow(val_2, x, &tmp);
    val_2 = tmp;
    overflow_2 |= __builtin_add_overflow(val_2, x_2, &tmp);
    val_2 = tmp;
    overflow_2 |= __builtin_add_overflow(val_2, x_3, &tmp);
    val_2 = tmp;
    overflow_2 |= __builtin_mul_overflow(val_2, block_size, &tmp);
    val_2 = tmp;

//    printf("%d: %lld, %d: %lld\n", overflow_1, val_1, overflow_2, val_2);
    if (overflow_1 == 0 && overflow_2 == 0) {
        return MIN(val_1, val_2);
    } else if (overflow_1 == 0 && overflow_2 == 1) {
        return val_1;
    } else if (overflow_1 == 1 && overflow_2 == 0) {
        return val_2;
    } else {
        return -1;
    }
}

int
main()
{
    int64_t block_size, block_num_size, inode_direct_block_count;
    scanf("%" PRId64 "%" PRId64 "%" PRId64, &block_size, &block_num_size, &inode_direct_block_count);

    printf("%" PRId64 "\n", calc_max_size(block_size, block_num_size, inode_direct_block_count));

    int64_t overflow = 0, best_num = -1, max_size = INT64_MIN;
    for (int64_t i = 1; !overflow && i <= block_size; i++) {
        int64_t res = calc_max_size(block_size, i, inode_direct_block_count);
        if (res == -1) {
            overflow = 1;
            max_size = -1;
            best_num = i;
        }
        if (res > max_size) {
            max_size = res;
            best_num = i;
        }
    }
    printf("%" PRId64 " %" PRId64 "\n", best_num, max_size);
    return 0;
}




