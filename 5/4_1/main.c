#include <string.h>
int
parse_rwx_permissions(const char *str) {
    if(!str)
        return -1;
    const char sample[] = "rwxrwxrwx";
    int len_sample = sizeof(sample), len_str = strlen(str);
    if (len_sample - 1 != len_str)
        return -1;

    int ans = 0;
    for (int i = 0; i < len_str; i++) {
        int bit;
        if (str[i] == sample[i]) {
            bit = 1;
        } else {
            if (str[i] == '-') {
                bit = 0;
            } else {
                return -1;
            }
        }
        ans = (ans << 1) + bit;
    }
    return ans;
}

