#include <stdio.h>
enum Result {
    reject,
    summon,
    disqualify,
};

int main(int argc, char **argv) {
    int x;
    sscanf(argv[1], "%d", &x);
    enum Result res = (%s);
    switch(res) {
        case reject:
            printf("reject\n");
            break;
        case summon:
            printf("summon\n");
            break;
        case disqualify:
            printf("disqualify\n");
            break;
        default:
            break;
    }
    return 0;
}
