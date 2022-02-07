#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <unistd.h>

unsigned f(void *point, unsigned block_size) {
    unsigned ans = 0;
    unsigned *p = (unsigned *) ans;
    for (int i = 0; i < 10; i++) {
        if (((unsigned *) point)[i] == 0) {
            return ans;
        }
        ans++;
    }
    unsigned count_elem = block_size / sizeof(int);
    for (int i = 0; i < count_elem; i++) {
        if (((unsigned **) point)[10][i] == 0) {
            return ans;
        }
        ans++;
    }
    for (int i = 0; i < count_elem; i++) {
        if (((unsigned **) point)[11][i] == 0) {
            return ans;
        }
        for (int j = 0; j < count_elem; j++) {

            if (((unsigned ***) point)[11][i][j] == 0) {
                return ans;
            }
            ans++;
        }
    }
    for (int i = 0; i < count_elem; i++) {
        if (((unsigned **) point)[12][i] == 0) {
            return ans;
        }
        for (int j = 0; j < count_elem; j++) {
            if (((unsigned ***) point)[12][i][j] == 0) {
                return ans;
            }
            for (int k = 0; k < count_elem; k++) {
                if (((unsigned ****) point)[12][i][j][k] == 0) {
                    return ans;
                }
                ans++;
            }
        }
    }
    return ans;// если все занято
}
