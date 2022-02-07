#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <iostream>

using namespace std;
struct bhnode { // node
    //long long data;
    long long priority;
};

struct binary_heap {
    bhnode *body;
    long long bodysize;
    long long numnodes;
    long long direction;

    binary_heap(long long maxsize) {
        direction = -1;
        body = new bhnode[maxsize+1];
        bodysize = maxsize;
        numnodes = 0;
    }

    ~binary_heap() {
        delete body;
    }

    void swap(long long a, long long b) {
        std::swap(body[a],body[b]);
    }

    bhnode * fetchTop() {
        if (numnodes <= 0) return NULL;
        return {&body[1]};
    }

    long long insert(bhnode node) {
        if (numnodes > bodysize) {
            return -1; // or expand
        }
        body[++numnodes] = node;
        for (size_t i = numnodes;
             i > 1 && body[i].priority < body[i/2].priority;
             i /= 2) {
            swap(i, i/2);
        }
        return 0;
    }

    void delete_first() {
        swap (1, numnodes--);
        heapify(1);
    }

    void heapify(size_t index) {
        for (;;) {
            auto left = index + index, right = left + 1;
            // Who is greater, [index], [left], [right]?
            auto largest = index;
            if (left <= numnodes && body[left].priority < body[index].priority)
                largest = left;
            if (right <= numnodes && body[right].priority < body[largest].priority)
                largest = right;
            if (largest == index)
                break;
            swap(index, largest);
            index = largest;
        }
    }

    void print(long long index = 1, long long spaces = 0) {
        if (index > numnodes) return;
        print(index * 2, spaces + 1);

        for (long long i = 0; i < spaces; i++) {
            printf("\t");
        }
        printf("%d\n", body[index].priority);
        print(index * 2 + 1, spaces + 1);
    }
};