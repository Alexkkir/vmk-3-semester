//#include <stdio.h>
#include <stdlib.h>


typedef struct RandomGenerator RandomGenerator;
typedef struct RandomOperations RandomOperations;

struct RandomOperations {
    int (*next)(RandomGenerator *);
    void (*destroy)(RandomGenerator *);
};

struct RandomGenerator {
    RandomOperations *ops;
    unsigned long long a, c, m, x;
};

int next(RandomGenerator *rr) {
    unsigned long long next = (rr->x * rr->a + rr->c) % rr->m;
    rr->x = next;
    return (int) next;
}

void destroy(RandomGenerator *rr) {
    free(rr->ops);
    free(rr);
}

RandomGenerator *random_create(int seed) {
    RandomGenerator *rr = calloc(1, sizeof(RandomGenerator));
    rr->x = seed;
    rr->a = 1103515245;
    rr->c = 12345;
    rr->m = 1ull << 31;
    rr->ops = calloc(1, sizeof(RandomOperations));
    rr->ops->next = &next;
    rr->ops->destroy = &destroy;
    return rr;
}
//
//int main() {
//    RandomGenerator *rr = random_create(1234);
//    for (int j = 0; j < 100; ++j) {
//        printf("%d\n", rr->ops->next(rr));
//    }
//    rr->ops->destroy(rr);
//    return 0;
//}
