#include <stdio.h>
#include <string.h>

int
main(void)
{
    char s1[200], s2[200], s3[200];
    fscanf(stdin, "%s%s%s", s1, s2, s3);
    fprintf(stdout, "[Host:%s,Login:%s,Password:%s]\n", s1, s2, s3);
    return 0;
}
