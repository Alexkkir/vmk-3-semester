#include <iostream>

class ArrayLeft {
    int maxsize; // размер выделенной области памяти body. Максимальная длина массива <= maxsize
    int len; // текущая длина массива. В начале = 0
public: // делаем методы публичными
    int *body; // область памяти, выделенная для хранения массива

    ArrayLeft(int maxsize) : maxsize(maxsize), len(0)
    {
        body = new int[maxsize]; // выделяем область памяти, внутри которой будет располагаться массив
    }

    void
    add_first(int x)
    {
        for (int i = len - 1; i >= 0; i--) // сдвигаем все элементы направо на 1
            body[i + 1] = body[i];
        body[0] = x;    // присваиваем значение первому элементу массива (начало массива совпадает с началом областью
        // памяти)
        len++; // добавили элемент => длина увеличилась
    }

    void
    del_first()
    {
        for (int i = 0; i < len - 1; i++) { // сдвигаем все элементы налево на 1
            body[i] = body[i + 1];
        }
        len--; // добавили элемент => длина уменьшилась
    }

    void
    add_end(int x)
    {
        body[len] = x;  // присваиваем последнему элементу массива значение
        len++;
    }

    void
    del_end()
    {
        len--;
    }
};

class ArrayRight {
    // в целом, принцип работы такой же, как у ArrayLeft
    int maxsize;
    int first_pos;
public:
    int *body;

    ArrayRight(int maxsize) : maxsize(maxsize), first_pos(maxsize - 1)
    {
        body = new int[maxsize];
    }

    void
    add_first(int x)
    {
        body[first_pos] = x;
        first_pos--;
    }

    void
    del_first()
    {
        first_pos++;
    }

    void
    add_end(int x)
    {
        for (int i = first_pos; i < maxsize; i++) // сдвигаем все элементы налево на 1
            body[i] = body[i + 1];
        body[maxsize - 1] = x;
        first_pos--;
    }

    void
    del_end()
    {
        for (int i = maxsize - 1; i > first_pos; i--) // сдвигаем все элементы направо на 1
            body[i] = body[i - 1];
        first_pos++;
    }
};


class ArrayCenter {
    int maxsize;
    int first_pos, last_pos;
public:
    // таким образом, весь массив лежит в полуинтервали [first_pos, last_pos)
    // last_pos - это индекс последнего элемента массива плюс один
    // first_pos - индекс первого элемента массива
    int *body;
    ArrayCenter(int maxsize) : maxsize(maxsize), first_pos(maxsize / 2), last_pos(maxsize / 2)
    {
        body = new int[maxsize];
    }

    /*
     палочки | и | - это указатели first_pos и last_pos

     O O O O O O O O O O        // в начале
             ||

     O O O X O O O O O O        // add_first(X)
           | |

     O O O O X O O O O O        // add_last(X)
             | |
     */

    void
    add_first(int x)
    {
        first_pos--;
        body[first_pos] = x;
    }

    void
    del_first()
    {
        first_pos++;
    }

    void
    add_end(int x)
    {
        body[last_pos] = x;
        last_pos++;
    }

    void
    del_end()
    {
        last_pos--;
    }
};

/*
    Асимптотики ArrayLeft
    add_first O(n)
    del_first O(n)
    add_end O(1)
    del_end O(1)

    Асимптотики ArrayRight
    add_first O(1)
    del_first O(1)
    add_end O(n)
    del_end O(n)

    Асимптотики ArrayCenter
    add_first O(1)
    del_first O(1)
    add_end O(1)
    del_end O(1)
 */

int
main()
{
    std::cout << "Testing ArrayLeft" << std::endl;
    ArrayLeft arr_left = ArrayLeft(10);
    arr_left.add_first(2);
    arr_left.add_end(3);
    arr_left.add_first(1);
    arr_left.add_end(4);

    for (int i = 0; i < 4; i++) {
        std::cout << arr_left.body[i] << std::endl; // 1 2 3 4
    }
    std::cout << std::endl;

    arr_left.del_first();
    arr_left.del_end();


    for (int i = 0; i < 2; i++) {
        std::cout << arr_left.body[i] << std::endl; // 2 3
    }


    std::cout << "Testing ArrayRight" << std::endl;
    ArrayRight arr_right = ArrayRight(10);
    arr_right.add_first(2);
    arr_right.add_end(3);
    arr_right.add_first(1);
    arr_right.add_end(4);

    for (int i = 6; i < 10; i++) {
        std::cout << arr_right.body[i] << std::endl; // 1 2 3 4
    }
    std::cout << std::endl;

    arr_right.del_first();
    arr_right.del_end();


    for (int i = 8; i < 10; i++) {
        std::cout << arr_right.body[i] << std::endl; // 2 3
    }


    std::cout << "Testing ArrayCenter" << std::endl;
    ArrayCenter arr_center = ArrayCenter(10);
    arr_center.add_first(2);
    arr_center.add_end(3);
    arr_center.add_first(1);
    arr_center.add_end(4);

    for (int i = 3; i < 7; i++) {
        std::cout << arr_center.body[i] << std::endl; // 1 2 3 4
    }
    std::cout << std::endl;

    arr_center.del_first();
    arr_center.del_end();


    for (int i = 4; i < 6; i++) {
        std::cout << arr_center.body[i] << std::endl; // 2 3
    }

    return 0;
}
