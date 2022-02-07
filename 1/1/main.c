#include <stdio.h>
#include <stdlib.h>

enum {
    len = 109
};

int
main(void)
{
    int v, t;
    scanf("%d%d", &v, &t);
    int ans = ((v * t % len) + len) % len;
    printf("%d", ans);
    return 0;
}