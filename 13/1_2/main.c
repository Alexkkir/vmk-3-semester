#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

enum { INIT_VAL = -1, };

int
main(int argc, char **argv)
{
    int memory_size, cache_size, block_size, fd;
    {
        int *addr[] = {&memory_size, &cache_size, &block_size};
        for (int i = 0; i < 3; i++) {
            sscanf(argv[1 + i], "%d", addr[i]);
        }
    }
    if (argc == 5) {
        fd = open(argv[4], 0);
        dup2(fd, 0);
    }

    int cache_elems = cache_size / block_size;
    int *cache = malloc(cache_elems * sizeof(int));
    for (int i = 0; i < cache_elems; i++) {
        cache[i] = INIT_VAL;
    }

    int misses = 0;

    char op;
    int addr, size, value;
    char line[4];
    while (scanf("%s%x%d%d", line, &addr, &size, &value) > 0) {
        op = line[0];
//        printf("> %c %x %d %d\n", op, addr, size, value);
        int memory_index = addr / block_size;
        int cache_index = memory_index % cache_elems;
        if (op == 'W') {
            cache[cache_index] = memory_index;
        } else if (op == 'R') {
            int stored_index = cache[cache_index];
            if (stored_index != memory_index && stored_index != INIT_VAL) {
                misses++;
            }
            cache[cache_index] = memory_index;
        }
    }

    printf("%d\n", misses);
    fflush(stdout);
    free(cache);
    return 0;
}
