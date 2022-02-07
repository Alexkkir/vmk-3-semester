#include <cstdlib>
#include <set>
#include <unordered_set>
#include <stdio.h>
#include <string>
#include <time.h>
#include <iostream>
#include <vector>


#define NDEBUG 1    // Все assert - просто комментарии

#include <cassert>
#include "crc.h"

using namespace std;

class FastRandomHT {
private:
    unsigned long long rnd;

public:
    FastRandomHT(unsigned long long seed = 0x38C176D1)
    { rnd = seed; }

    void
    seed(unsigned long long seed)
    { rnd = seed; }

    unsigned long long
    rand()
    {
        rnd ^= rnd << 21;
        rnd ^= rnd >> 35;
        rnd ^= rnd << 4;
        return rnd;
    }
};


template<typename T>
struct HT {
    HT(size_t initial_size)
    {
        body.resize(initial_size);
        hashsize = initial_size;
    }

    ~HT()
    {
//      cout << "hits= " << hits << "\nmisses= " << misses << "\nsearches= " << searches << "\n";
    }

    void
    print() const
    {
        for (auto const &q: body) {
            if (q.st == USED) {
                cout << q.key << endl;
            }
        }
    }

    bool
    insert(T const &_key)
    {
        if (find(_key)) return false;
        if (used + used >= hashsize) {
            reorder();
        }
        //auto h = hasher.calc((const char *)_key.c_str(), _key.size()) % hashsize; // Для строки!!!!!!
        auto h = hasher.calc((const char *) &_key, sizeof _key) % hashsize;
        // auto h = _key % hashsize;
        for (;;) {
            searches++;
            if (body[h].st == EMPTY || body[h].st == DELETED) {
                body[h] = {_key, USED};
                used++;
                hits++;
                return true;
            }
            h = (h + 1) % hashsize;
        }
        return true;
    }

    bool
    find(T const &_key) const
    {
        // auto h = hasher.calc((const char *)_key.c_str(), _key.size()) % hashsize; // Для строки!!!!!!
        auto h = hasher.calc((const char *) &_key, sizeof _key) % hashsize;
        //auto h = _key % hashsize;
        for (;;) {
            searches++;
            if (body[h].st == EMPTY) return false;
            if (body[h].st == USED && body[h].key == _key) {
                hits++;
                return true;
            }
            h = (h + 1) % hashsize;
        }
        return false;
    }

    bool
    remove(T const &_key)
    {
        // auto h = hasher.calc((const char *)_key.c_str(), _key.size()) % hashsize; // Для строки!!!!!!
        auto h = hasher.calc((const char *) &_key, sizeof _key) % hashsize;
        //auto h = _key % hashsize;
        for (;;) {
            searches++;
            if (body[h].st == EMPTY) return false;
            if (body[h].st == USED && body[h].key == _key) {
                body[h].st = DELETED;
                used--;
                hits++;
                return true;
            }
            h = (h + 1) % hashsize;
        }
        return true;
    }

    void
    reorder()
    {
        throw "!!!!!";
    }

    enum status {
        EMPTY, USED, DELETED
    };
    struct key_s {
        T key;
        status st = EMPTY;
    };
    vector<key_s> body;
    size_t hashsize;
    size_t used = 0;
    CRCCoder<unsigned> hasher;
    mutable size_t hits = 0, misses = 0, searches = 0;
};

#if 0
using keytype = std::string;

int main() {
  HT<keytype> t(23);
  t.insert("abra");
  t.insert("cadabra");
  t.insert("foo");
  t.insert("bar");
  t.print();
  t.remove("cadabra");
  t.print();
}
#else
using keytype = unsigned long long;

void
test_HT()
{
    const int SIZE = 1000001;
    FastRandom r;
    r.seed(time(NULL));
    vector<keytype> pat;
    vector<keytype> search_pat;
    for (int i = 0; i < SIZE; i++) {
        pat.push_back(i);
    }
    for (int i = 0; i < SIZE * 5; i++) {
        int num = i % (SIZE * 2);
        if (num < SIZE) {
            search_pat.push_back(pat[num]);
        } else {
            search_pat.push_back(i * 10);
        }
    }
    HT<keytype> t(SIZE * 3 + 4);
    clock_t s1 = clock();
    for (auto const &s: pat) {
        t.insert(s);
    }
    clock_t s1_1 = clock();
    int count = 0;
    for (auto const &s: search_pat) {
        auto res = t.find(s);
        count += res;
        //printf("%d\n", res);
    }
    //t.print();
    printf("HT: count=%d\n", count);
    clock_t s1_2 = clock();
    for (auto const &s: pat) {
        t.remove(s);
    }
    clock_t e1 = clock();

    unordered_set<keytype> se1;
    clock_t s2 = clock();
    for (auto const &s: pat) {
        se1.insert(s);
    }
    clock_t s2_1 = clock();
    count = 0;
    for (auto const &s: search_pat) {
        auto res = se1.find(s);
        count += res != se1.end();
    }
    printf("SET: count=%d\n", count);
    clock_t s2_2 = clock();
    for (auto const &s: pat) {
        se1.erase(s);
    }
    clock_t e2 = clock();

    printf("HT: insert=%.3f find=%.3f erase=%.3f\n"
           "SET: insert=%.3f find=%.3f erase=%.3f\n",
           (double) (s1_1 - s1) / CLOCKS_PER_SEC,
           (double) (s1_2 - s1_1) / CLOCKS_PER_SEC,
           (double) (e1 - s1_2) / CLOCKS_PER_SEC,
           (double) (s2_1 - s2) / CLOCKS_PER_SEC,
           (double) (s2_2 - s2_1) / CLOCKS_PER_SEC,
           (double) (e2 - s2_2) / CLOCKS_PER_SEC);
}

#endif
