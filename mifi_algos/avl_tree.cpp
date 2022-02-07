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

using namespace std;

class FastRandom {
private:
    unsigned long long rnd;

public:
    FastRandom(unsigned long long seed = 0x38C176D1)
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
struct AVLNode {
    AVLNode *left, *right;
    T key;
    int height;

    AVLNode(T const &_key) : key(_key)
    {
        left = right = nullptr;
        height = 1;
    }

    int
    getBalanceFactor() const
    {
        int r = right == nullptr ? 0 : right->height;
        int l = left == nullptr ? 0 : left->height;
        return r - l;
    }

    void
    fix()
    {
        int r = right == nullptr ? 0 : right->height;
        int l = left == nullptr ? 0 : left->height;
        height = (r > l ? r : l) + 1;
    }

    AVLNode *
    insert(T const &_key)
    {
        if (_key < key)
            left = left == nullptr ? new AVLNode(_key) : left->insert(_key);
        else
            right = right == nullptr ? new AVLNode(_key) : right->insert(_key);
        return balance();
    }

    AVLNode *
    findMinimum()
    {
        return left != nullptr ? left->findMinimum() : this;
    }

    AVLNode *
    removeMinimum()
    {
        if (left == nullptr)
            return right;
        left = left->removeMinimum();
        return balance();
    }

    AVLNode *
    rotateRight()
    {
        auto t = left;
        left = t->right;
        t->right = this;
        this->fix();
        t->fix();
        return t;
    }

    AVLNode *
    rotateLeft()
    {
        auto t = right;
        right = t->left;
        t->left = this;
        this->fix();
        t->fix();
        return t;
    }

    AVLNode *
    balance()
    {
        fix();
        switch (getBalanceFactor()) {
            case 2:
                if (right->getBalanceFactor() < 0) {
                    right = right->rotateRight();
                }
                return rotateLeft();
            case -2:
                if (left->getBalanceFactor() > 0) {
                    left = left->rotateLeft();
                }
                return rotateRight();
            default:
                return this;
        }
    }

    static AVLNode *
    remove(AVLNode *p, T const &_key)
    {
        if (p == nullptr)
            return nullptr;
        if (_key < p->key) {
            p->left = remove(p->left, _key);
            return p->balance();
        } else if (_key > p->key) {
            p->right = remove(p->right, _key);
            return p->balance();
        } else {
            AVLNode *l = p->left;
            AVLNode *r = p->right;
            delete p;
            if (r == nullptr)
                return l;
            AVLNode *min = r->findMinimum();
            min->right = r->removeMinimum();
            min->left = l;
            return min->balance();
        }
    }

    void
    print(int lev) const
    {
        if (right != nullptr) right->print(lev + 1);
        for (int i = 0; i < lev; i++) {
            cout << "  ";
        }
        cout << key << endl;
        if (left != nullptr) left->print(lev + 1);
    }

};

template<typename T>
struct AVL {
    AVLNode<T> *root;

    AVL()
    { root = nullptr; }

    void
    print() const
    {
        if (root != nullptr)
            root->print(0);
    }

    bool
    insert(T const &_key)
    {
        if (root == nullptr)
            root = new AVLNode<T>(_key);
        else
            root = root->insert(_key);
        return true;
    }

    bool
    find(T const &_key) const
    {
        auto t = root;
        while (t != nullptr) {
            if (t->key == _key) return true;
            t = _key <= t->key ? t->left : t->right;
        }
        return false;
    }

    bool
    remove(T const &_key)
    {
        root = AVLNode<T>::remove(root, _key);
        return true;
    }

    static void
    test(int type)
    {
        if (type == 1) {
            AVL<string> t;
            t.insert("abra");
            t.insert("cadabra");
            t.insert("foo");
            t.insert("bar");
            t.print();
            t.remove("cadabra");
            t.print();
        } else if (type == 2){
            const int SIZE = 1000001;
            FastRandom r;
            r.seed(time(NULL));
            vector<unsigned long long> pat;
            vector<unsigned long long> search_pat;
            for (int i = 0; i < SIZE; i++) {
                pat.push_back(r.rand());
            }
            for (int i = 0; i < SIZE * 5; i++) {
                int num = r.rand() % (SIZE * 2);
                if (num < SIZE) {
                    search_pat.push_back(pat[num]);
                } else {
                    search_pat.push_back(r.rand());
                }
            }
            AVL<unsigned long long> t;
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
            printf("AVL: count=%d\n", count);
            clock_t s1_2 = clock();
            for (auto const &s: pat) {
                t.remove(s);
            }
            clock_t e1 = clock();

            unordered_multiset<unsigned long long> se1;
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

            printf("AVL: insert=%.3f find=%.3f erase=%.3f\n"
                   "SET: insert=%.3f find=%.3f erase=%.3f\n",
                   (double) (s1_1 - s1) / CLOCKS_PER_SEC,
                   (double) (s1_2 - s1_1) / CLOCKS_PER_SEC,
                   (double) (e1 - s1_2) / CLOCKS_PER_SEC,
                   (double) (s2_1 - s2) / CLOCKS_PER_SEC,
                   (double) (s2_2 - s2_1) / CLOCKS_PER_SEC,
                   (double) (e2 - s2_2) / CLOCKS_PER_SEC);
        }
    }
};

