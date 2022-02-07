#include <algorithm>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <time.h>

using namespace std;

void
sort_heap(int *a, int n);

const int QTHRESHOLD = 50;

void
sort_bubble(int *a, int n)
{
    bool sorted = false;
    while (!sorted) {
        sorted = true;
        for (int i = 0; i < n - 1; i++) {
            if (a[i] > a[i + 1]) {
                int tmp = a[i];
                a[i] = a[i + 1];
                a[i + 1] = tmp;
                sorted = false;
            }
        }
    }
}

void
sort_shaker(int *a, int n)
{
    bool sorted = false;
    int l = 0, r = n;
    while (!sorted) {
        sorted = true;
        for (int i = l; i < r - 1; i++) {
            if (a[i] > a[i + 1]) {
                int tmp = a[i];
                a[i] = a[i + 1];
                a[i + 1] = tmp;
                sorted = false;
            }
        }
        r--;
        if (sorted)
            break;
        sorted = true;
        for (int i = r - 2; i >= l; i--) {
            if (a[i] > a[i + 1]) {
                int tmp = a[i];
                a[i] = a[i + 1];
                a[i + 1] = tmp;
                sorted = false;
            }
        }
        l++;
    }
}

void
sort_bubble_traditional(int *a, int n)
{
    bool sorted = false;
    for (int i = n - 1; --n > -0;) {
        for (int j = 0; j < i; j++) {
            if (a[j] > a[j + 1]) {
                int tmp = a[j];
                a[j] = a[j + 1];
                a[j + 1] = tmp;
            }
        }
    }
}

void
sort_insertion(int *a, int n)
{
    for (int i = n - 1; i > 0; i--) {
        if (a[i - 1] > a[i]) {
            int tmp = a[i - 1];
            a[i - 1] = a[i];
            a[i] = tmp;
        }
    }
    for (int i = 2; i < n; i++) {
        int j = i;
        int tmp = a[i];
        while (tmp < a[j - 1]) {
            a[j] = a[j - 1];
            j--;
        }
        a[j] = tmp;
    }
}

void
sort_shell(int *a, int n)
{
    int h;
    for (h = 1; h <= n / 9; h = 3 * h + 1);
    for (; h > 0; h /= 3) {
        for (int i = h; i < n; i++) {
            int j = i;
            int tmp = a[i];
            while (j >= h && tmp < a[j - h]) {
                a[j] = a[j - h];
                j -= h;
            }
            a[j] = tmp;
        }
    }
}

void
sort_shell2(int *a, int n)
{
    static long long steps[] = {1,
                                8,
                                23,
                                77,
                                281,
                                1073,
                                4193,
                                16577,
                                65921,
                                262913,
                                1050113,
                                4197377,
                                16783361,
                                67121153,
                                268460033,
                                1073790977,
                                4295065601,
                                17180065793,
                                68719869953,
                                274878693377,
                                1099513200641,
                                4398049656833,
                                17592192335873,
                                70368756760577};
    int s;
    for (s = 0; steps[s] < n; s++);
    for (; --s >= 0;) {
        int h = steps[s];
        for (int i = h; i < n; i++) {
            int j = i;
            int tmp = a[i];
            while (j >= h && tmp < a[j - h]) {
                a[j] = a[j - h];
                j -= h;
            }
            a[j] = tmp;
        }
    }
}

void
sort_shell3(int *a, int n)
{
    int h = n / 2;
    for (; h > 0; h /= 2) {
        for (int i = h; i < n; i++) {
            int j = i;
            int tmp = a[i];
            while (j >= h && tmp < a[j - h]) {
                a[j] = a[j - h];
                j -= h;
            }
            a[j] = tmp;
        }
    }
}

void
sort_shell4(int *a, int n)
{
    static long long steps[] = {
            1, 3, 7, 21, 48, 112,
            336, 861, 1968, 4592, 13776, 33936,
            86961, 198768, 463792, 1391376, 3402672, 8382192,
            21479367, 49095696, 114556624, 343669872, 852913488, 2085837936,
            5138283696, 13166851971, 30095661648, 70223210512};
    int s;
    for (s = 0; steps[s] < n; s++);
    for (; --s >= 0;) {
        int h = steps[s];
        for (int i = h; i < n; i++) {
            int j = i;
            int tmp = a[i];
            while (j >= h && tmp < a[j - h]) {
                a[j] = a[j - h];
                j -= h;
            }
            a[j] = tmp;
        }
    }
}

void
sort_comb(int *a, int n)
{
    double s = n - 1;
    while (s >= 1) {
        int d = s;
        for (int i = d; i < n; i++) {
            if (a[i - d] > a[i]) {
                swap(a[i - d], a[i]);
            }
        }
        s /= 1.24733;
    }
}

void
merge(int *a, int low, int mid, int high, int *aux)
{
    int i, j;
    for (i = mid + 1; i > low; i--)
        aux[i - 1] = a[i - 1];
    for (j = mid; j < high; j++)
        aux[high + mid - j] = a[j + 1];
    for (int k = low; k <= high; k++) {
        if (aux[j] < aux[i])
            a[k] = aux[j--];
        else
            a[k] = aux[i++];
    }
}

void
mergeSort(int a[], int low, int high, int *aux)
{
    const int THRESHOLD = QTHRESHOLD;
    if (high - low < THRESHOLD) {
        sort_insertion(a + low, high - low + 1);
    } else {
        int mid = (low + high) / 2;
        mergeSort(a, low, mid, aux);
        mergeSort(a, mid + 1, high, aux);
        merge(a, low, mid, high, aux);
    }
}

void
sort_merge(int *a, int n)
{
    int *aux = new int[n];
    mergeSort(a, 0, n - 1, aux);
    delete[] aux;
}

void
quickSortBase(int *a, int l, int r)
{
    if (r - l < QTHRESHOLD) {
        sort_insertion(a + l, r - l + 1);
        return;
    }
    int i = l, j = r;
    int aa = a[l], bb = a[r], cc = a[(l + r) >> 1];
    int p = aa;
    if (aa < bb) {
        if (bb < cc) {
            p = bb;
        } else {
            if (aa < cc) {
                p = cc;
            } else {
                p = aa;
            }
        }
    } else {
        if (bb < cc) {
            if (aa < cc) {
                p = aa;
            } else {
                p = cc;
            }
        } else {
            p = bb;
        }
    }
    while (i <= j) {
        while (p > a[i])
            i++;
        while (a[j] > p)
            j--;
        if (i <= j) {
            int tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
            i++;
            j--;
        }
    }
    if (l < j)
        quickSortBase(a, l, j);
    if (i < r)
        quickSortBase(a, i, r);
}

void
sort_quick_recursive(int *a, int n)
{ quickSortBase(a, 0, n - 1); }

void
sort_quick_iterative(int *a, int size)
{
    const int MAXSTACK = 2048;
    const int THRESHOLD = 50;
    int i, j;
    int lb, ub;
    int lbstack[MAXSTACK], ubstack[MAXSTACK];
    int stackpos = 1;
    int ppos;
    int pivot;
    int temp;

    lbstack[1] = 0;
    ubstack[1] = size - 1;

    do {
        lb = lbstack[stackpos];
        ub = ubstack[stackpos];
        stackpos--;
        if (ub - lb < THRESHOLD) {
            sort_insertion(a + lb, ub - lb + 1);
            continue;
        }

        do { // Hoar partition
            ppos = (lb + ub) >> 1;
            i = lb;
            j = ub;
            pivot = a[ppos];

            do {
                while (a[i] < pivot)
                    i++;
                while (pivot < a[j])
                    j--;
                if (i <= j) {
                    std::swap(a[i], a[j]);
                    i++;
                    j--;
                }
            } while (i <= j);

            if (i < ppos) {
                if (i < ub) {
                    stackpos++; //  сортировать, запрос в стек
                    lbstack[stackpos] = i;
                    ubstack[stackpos] = ub;
                }
                ub = j;
            } else { // левая часть больше
                if (j > lb) {
                    stackpos++;
                    lbstack[stackpos] = lb;
                    ubstack[stackpos] = j;
                }
                lb = i;
            }
        } while (lb < ub); // пока в меньшей части более 1 элемента

    } while (stackpos != 0); // пока есть запросы в стеке
}

static void
heapify(int *a, int i, int n)
{
    int curr = a[i];
    int index = i;
    for (;;) {
        int left = index + index + 1;
        int right = left + 1;
        if (left < n && a[left] > curr)
            index = left;
        if (right < n && a[right] > a[index])
            index = right;
        if (index == i)
            break;
        a[i] = a[index];
        a[index] = curr;
        i = index;
    }
}

void
sort_heap(int *a, int n)
{
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(a, i, n);
    }
    while (n > 1) {
        n--;
        int tmp = a[0];
        a[0] = a[n];
        a[n] = tmp;
        heapify(a, 0, n);
    }
}

void
sort_exchange(int *a, int n)
{
    for (int i = 0; i < n - 1; i++) {
        int imin = i;
        for (int j = i + 1; j < n; j++) {
            if (a[j] < a[imin])
                imin = j;
        }
        int tmp = a[imin];
        a[imin] = a[i];
        a[i] = tmp;
    }
}

void
sort_vector(int *a, int n)
{ sort(a + 0, a + n); }

void
sort_vector_stable(int *a, int n)
{ stable_sort(a + 0, a + n); }

int
compare_int(const void *a, const void *b)
{
    return *(const int *) a - *(const int *) b;
}

void
sort_qsort(int *a, int n)
{ qsort(a, n, sizeof(int), compare_int); }

typedef void (*sort_func)(int *a, int n);

struct sort_s {
    sort_func func;
    string name;
} funcs[] = {{sort_bubble,             "bubble_simple"},
             {sort_bubble_traditional, "bubble_trad"},
             {sort_shaker,             "shaker"},
             {sort_insertion,          "insertion"},
             {sort_exchange,           "exchange"},
             {sort_shell,              "shell"},
             {sort_shell2,             "shell2"},
             {sort_shell3,             "shell3"},
             {sort_shell4,             "shell4"},
             {sort_comb,               "comb"},
             {sort_merge,              "merge"},
             {sort_heap,               "heap"},
             {sort_quick_recursive,    "quick recursive"},
             {sort_quick_iterative,    "quick iterative"},
        //     {sort_quick_lomuto, "quick lomuto"},
        //     {sort_tim, "timsort"},
             {sort_vector,             "vector"},
             {sort_vector_stable,      "vector stable"},
             {sort_qsort,              "qsort"},
        //     {sort_radix4, "radix4"},
        //     {sort_radix8, "radix8"},
        //     {sort_radix16, "radix16"}
};

int
test_sortings(int argc, char **argv)
{
    string kind = "random";
    if (argc > 1 && argv[1] == string("random")) {
        kind = "random";
        argc--;
        argv++;
    }
    if (argc > 1 && argv[1] == string("ascend")) {
        kind = "ascend";
        argc--;
        argv++;
    }
    if (argc > 1 && argv[1] == string("descend")) {
        kind = "descend";
        argc--;
        argv++;
    }
    int start_alg = 0;
    if (argc > 1 && argv[1] == string("fastest")) {
        argc--;
        argv++;
        start_alg = 5;
    }
    int n = 100000;
    if (argc > 1) {
        n = atoi(argv[1]);
    }
    int *copya = new int[n];
    if (kind == "random") {
        srand(time(NULL));
        for (int i = 0; i < n; i++) {
            copya[i] = rand();
        }
    } else if (kind == "ascend") {
        for (int i = 0; i < n; i++) {
            copya[i] = i * 2;
        }
    } else if (kind == "descend") {
        for (int i = 0; i < n; i++) {
            copya[i] = n * 2 - i * 2;
        }
    }
    int *a = new int[n];
    for (int alg = start_alg; alg < sizeof funcs / sizeof funcs[0]; alg++) {
        memcpy(a, copya, n * sizeof a[0]);
        printf("%30s: ", funcs[alg].name.c_str());
        fflush(stdout);
        clock_t start = clock();
        funcs[alg].func(a, n);
        clock_t end = clock();
        printf("%.3f seconds\n", (end - start) / (double) CLOCKS_PER_SEC);
        for (int i = 0; i < n - 1; i++) {
            if (a[i] > a[i + 1]) {
                printf("sort failed: a[%d]=%d a[%d]=%d\n", i, a[i], i + 1, a[i + 1]);
                break;
            }
        }
    }
    return 0;
}
