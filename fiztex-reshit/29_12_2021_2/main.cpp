#include <iostream>
#include <string>
#include <fstream>
#include <set>

using std::set;
using std::string;
using namespace std;

int
func(string filename)
{
    ifstream file;
    file.open(filename);
    if (!file.is_open()) {
        return 0;
    }
    int count = 0;
    set<int> good_vals = {1, 2, 3, 4, 5};

    int x;
    while (file >> x) {
        if (good_vals.find(x) != good_vals.end())
            count++;
    }
    return count;
}

int
main()
{
    string filename = "1.txt";
//    cin >> filename;
    cout << func(filename) << endl;
    return 0;
}
