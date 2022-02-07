#include <iostream>
#include <string>
#include <fstream>

using std::string;
using namespace std;

int
func(const string& filename)
{
    ifstream file;
    file.open(filename);

    if (!file.is_open()) {
        return 0;
    }

    enum State {
        UP, EQ, DOWN,
    } st;
    int len, len_max = 0;
    bool possible;

    int x_prev = 0, x;
    bool first = true;
    int last = 1;
    while (file >> x || last--) {
        if (first || x_prev < x) {
            first = false;
            st = UP;
        } else if (last == 0 || x_prev > x) {
            st = DOWN;
        } else if (x_prev == x) {
            st = EQ;
        }

        switch (st) {
            case UP:
                possible = true;
                len = 1;
                break;
            case EQ:
                len++;
                break;
            case DOWN:
                if (possible) {
                    if (len > len_max)
                        len_max = len;
                }
                possible = false;
                break;
        }
        x_prev = x;
    }
    return len_max;
}

int
main()
{
    string filename;
    cin >> filename;
    cout << func(filename) << endl;
    return 0;
}
