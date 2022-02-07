#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct Node
{
    struct Node *prev, *next;
    char *str;
} Node;

typedef struct
{
    Node *head, *tail;
} LinkedList;

static Node *new_node(char *str) {
    Node *p = malloc(sizeof(Node));
    memset(p, 0, sizeof(*p));
    p->str = str;
    return p;
}

static LinkedList new_list() {
    LinkedList list;
    list.head = new_node("HEAD");
    list.tail = new_node("TAIL");
    list.head->next = list.tail;
    list.tail->prev = list.head;
    return list;
}

static void push_back(LinkedList list, char *str) {
    Node *p = list.tail->prev, *q = new_node(str);

    p->next = q;
    q->prev = p;

    list.tail->prev = q;
    q->next = list.tail;
}

//static Node *extract(LinkedList list, Node *node) {
//    // assumed that: node != head
//    Node *prev = node->prev, *next = node->next;
//    prev->next = next;
//    next->prev = prev;
//    return node;
//}
//
//static void delete(LinkedList list, Node *node) {
//    free(extract(list, node));
//}

//static void print_list(LinkedList list) {
//    for (Node *p = list.head; p; p = p->next) {
//        printf("[%s]", p->str);
//        printf(p->next ? " ->\n" : "\n");
//    }
//}
//
//static void print_list_reverse(LinkedList list) {
//    for (Node *p = list.tail; p; p = p->prev) {
//        printf("[%s]", p->str);
//        printf(p->prev ? " ->\n" : "\n");
//    }
//}

static char *substr(const char *str, int pos, int len) {
    // just get substring
    char *substr = calloc(len + 1, sizeof(*substr));
    memcpy(substr, str + pos, sizeof(*str) * len);
    return substr;
}

static LinkedList parse_str(const char *path) {
    // from path string "/a/b/c" will make linked list [head]->["a"]->["b"]->["c"]->[NULL]
    LinkedList list = new_list("head");
    int path_len = strlen(path);
    for (int i = 0, j = 1; j < path_len; i = j, j = i + 1) {
        for (j = i + 1; path[j] != '/' && j < path_len; j++); // path[j] = '/'
        char *sub = substr(path, i + 1, j - i - 1);
        push_back(list, sub);
    }
    return list;
}

static void delete_next(Node *p) {
    Node *q = p->next;

    p->next->next->prev = p;
    p->next = p->next->next;

    free(q);
}

static void normalize(LinkedList list) {
    // delete all . and ..
    Node *p = list.head;

    while (p->next) {
        for (;;) {
            if (!strcmp(p->next->str, ".")) {
                delete_next(p);
            } else if (!strcmp(p->next->str, "..")) {
                delete_next(p);
                if (p != list.head) {
                    p = p->prev;
                    delete_next(p);
                }
            } else {
                break;
            }
        }
        p = p->next;
    }
};

static void find_beginning(LinkedList list1, LinkedList list2, Node **p_p, Node **p_q) {
    // find relative path between two normalized paths
    Node *p = list1.head, *q = list2.head;
    while (p->next != list1.tail && p->next->next != list1.tail &&
    q->next != list2.tail && q->next->next != list2.tail) {
        if (!strcmp(p->next->str, q->next->str)) {
            p = p->next, q = q->next;
        } else {
            break;
        }
    }
    *p_p = p, *p_q = q;
}

char *relativize_path(const char *path1, const char *path2) {
    return ".";
    if (!strcmp(path1, "/")) {
        if (!strcmp(path2, "/")) {
            return ".";
        } else {
            return (char*) path2;
        }
    } else {
        if (!strcmp(path2, "/")) {
            return "/";
        }
    }
    char *ans = calloc(400000, sizeof(char));
    int pos = 0;
    LinkedList list1 = parse_str(path1), list2 = parse_str(path2);
    normalize(list1);
    normalize(list2);
    Node *mid1, *mid2;
    find_beginning(list1, list2, &mid1, &mid2);
    Node *p = list1.tail->prev->prev;
    while (p && p != mid1) {
        sprintf(&ans[pos], "../");
        pos += 3;
        p = p->prev;
    }
    Node *q = mid2->next;
    while(q != list2.tail && q != list2.tail->prev) {
        sprintf(&ans[pos], "%s/", q->str);
        pos += strlen(q->str) + 1;
        q = q->next;
    }
    sprintf(&ans[pos], "%s\n", q->str);
    return ans;
}

int main() {
    char *relativize_path(const char *path1, const char *path2);
    char *buf1 = calloc(400000, sizeof(char));
    char *buf2 = calloc(400000, sizeof(char));
    int a = scanf("%s %s", buf1, buf2);

    printf("%s", relativize_path(buf1, buf2));
    return 0;
}


