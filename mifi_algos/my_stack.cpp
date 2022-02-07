/* C++ program to implement basic stack
   operations */
#include <bits/stdc++.h>

using namespace std;


class Stack {
private:
    int top;
    int maxsize;
    int *body;

public:
    Stack(int size = 100)
    {
        maxsize = size;
        body = new int[size];
        top = -1;
    }

    bool
    push(int x);

    int
    pop();

    int
    peek();

    bool
    isEmpty();

    static void
    test();
};

bool
Stack::push(int x)
{
    if (top >= (maxsize - 1)) {
        cout << "Stack Overflow";
        return false;
    } else {
        body[++top] = x;
        cout << x << " pushed into stack\n";
        return true;
    }
}

int
Stack::pop()
{
    if (top < 0) {
        cout << "Stack Underflow";
        return 0;
    } else {
        int x = body[top--];
        return x;
    }
}

int
Stack::peek()
{
    if (top < 0) {
        cout << "Stack is Empty";
        return 0;
    } else {
        int x = body[top];
        return x;
    }
}

bool
Stack::isEmpty()
{
    return (top < 0);
}

void
Stack::test()
{
    class Stack s;
    s.push(10);
    s.push(20);
    s.push(30);
    cout << s.pop() << " Popped from stack\n";
    //print all elements in stack :
    cout << "Elements present in stack : ";
    while (!s.isEmpty()) {
        // print top element in stack
        cout << s.peek() << " ";
        // remove top element in stack
        s.pop();
    }
}