#include <stdio.h>

unsigned *get_block(unsigned index);

unsigned func(const unsigned *inode_arr, unsigned block_size) {
    int count = 0;
    unsigned n = block_size / 4;

    // 1..10
    for (int i = 0; i < 10; i++) {
        if (inode_arr[i] != 0) {
            count++;
        } else {
            return count;
        }
    }
    if (inode_arr[10] != 0) {
//        unsigned *ptr = (unsigned *) &inode_arr[10];
        unsigned *ptr = get_block(inode_arr[10]);
        for (int i = 0; i < n; i++) {
            if (ptr[i] != 0) {
                count++;
            } else {
                return count;
            }
        }
    } else {
        return count;
    }

    if (inode_arr[11] != 0) {
        unsigned *ptr = (unsigned *) &inode_arr[11];
        for (int i = 0; i < n; i++) {
            if (ptr[i] != 0) {
                unsigned *ptr_2 = (unsigned *) &ptr[i];
                for (int j = 0; j < n; j++) {
                    if (ptr_2[j] != 0) {
                        count++;
                    } else {
                        return count;
                    }
                }
            } else {
                return count;
            }
        }
    } else {
        return count;
    }

    if (inode_arr[12] != 0) {
        unsigned *ptr_1 = (unsigned *) &inode_arr[12];
        for (int i = 0; i < n; i++) {
            if (ptr_1[i] != 0) {
                unsigned *ptr_2 = (unsigned *) &ptr_1[i];
                for (int j = 0; j < n; j++) {
                    if (ptr_2[j] != 0) {
                        unsigned *ptr_3 = (unsigned *) &ptr_2[j];
                        for (int k = 0; k < n; k++) {
                            if (ptr_3[k] != 0) {
                                count++;
                            } else {
                                return count;
                            }
                        }
                    } else {
                        return count;
                    }
                }
            } else {
                return count;
            }
        }
    } else {
        return count;
    }


    return count;
}

int main() {
    printf("Hello, World!\n");
    return 0;
}
