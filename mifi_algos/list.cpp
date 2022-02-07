#include <iostream>
#include <string>
#include <stdlib.h>

using namespace std;

class Node {
public:
    struct Node *prev, *next;
    string str;

    Node(string str_arg)
    {
        prev = nullptr;
        next = nullptr;
        str = str_arg;
    }
};

class DoubleLinkedList {
public:
    Node *head, *tail;

    DoubleLinkedList()
    {
        head = new Node("HEAD");
        tail = new Node("TAIL");
        head->next = tail;
        tail->prev = head;
    }

    void
    push_back(string str)
    {
        Node *p = tail->prev, *q = new Node(str);

        p->next = q;
        q->prev = p;

        tail->prev = q;
        q->next = tail;
    }

    Node *
    find(string str)
    {
        Node *p;
        for (p = head; p != tail && p->str.compare(str) != 0; p = p->next);
        if (p == tail)
            p = nullptr;
        return p;
    }

    Node *
    extract(Node *node)
    {
        // assumed that: node != head (it is normal case)
        Node *prev = node->prev, *next = node->next;
        prev->next = next;
        next->prev = prev;
        return node;
    }

    void
    delete_node(Node *node)
    {
        free(extract(node));
    }

    void
    print()
    {
        for (Node *p = head; p != nullptr; p = p->next) {
            cout << "[" << p->str << "]";
            if (p->next)
                cout << " ->";
            cout << "\n";
        }
    }

    static void
    delete_next(Node *p)
    {
        Node *q = p->next;
        p->next->next->prev = p;
        p->next = p->next->next;
        free(q);
    }

    static void
    test()
    {
        DoubleLinkedList list = DoubleLinkedList();
        list.push_back("abc");
        list.push_back("efg");
        list.print();
        cout << "abc in list: " << list.find("abc") << endl;
        cout << "efg " << list.find("efg") << endl;
        cout << "fff " << list.find("fff") << endl;
        list.delete_node(list.find("abc"));
        list.delete_node(list.find("efg"));
        list.print();
        list.push_back("aaabc");
        list.push_back("eeefg");
        list.print();
    }
};
