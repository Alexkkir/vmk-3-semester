#include <limits.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdint.h>

typedef long long ll;


typedef struct Node
{
    int32_t key;
    int32_t left_idx;
    int32_t right_idx;
} Node;

enum {
    MAX_N_NODES = 10000,
};

void
fun(int fd, int node_index)
{
    Node node;
    lseek(fd, sizeof(node) * node_index, SEEK_SET);

    int total = 0, tmp;
    while((tmp = read(fd, &node + total, sizeof(node) - total)) > 0) {
        total += tmp;
        if (total == sizeof(node))
            break;
    }

    if (node.right_idx) {
        fun(fd, node.right_idx);
    }
    printf("%d\n", node.key);
    if (node.left_idx) {
        fun(fd, node.left_idx);
    }
}

int comp (const int *i, const int *j)
{
    if (*i > 0 && *j < 0) {
        return 1;
    } else if (*i < 0 && *j > 0) {
        return -1;
    } else {
        return *j - *i;
    }
}

int
main(int argc, char **argv)
{
    int fd = open(argv[1], O_RDONLY);
    fun(fd, 0);
    return 0;
}