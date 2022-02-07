#include <stdio.h>

int main() {
    int cur_len = 0, str_size = 1024;
    int c;
    char *str = (char*) malloc (str_size * sizeof(char));
    while ((c = getchar()) != EOF && c != '\n') {
        str[cur_len] = (char) c;
        cur_len++;
        if (cur_len >= str_size) {
            str = realloc(str, 1024 + delta);
            str_size += delta;
        }
    }
    int result_len = min(cur_len, str_size);
    str[result_len] = '\0';
    puts(str);

    return 0;
    return 0;
}
