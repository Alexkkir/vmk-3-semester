#include <iostream>
#include <string>
#include <fstream>

using std::string;
using namespace std;

string
func(string filename)
{
    ifstream file;
    file.open(filename);
    if (!file.is_open()) {
        return "equals";
    }

    int sum[2] = {0, 0}, x;
    while(file >> x) {
        sum[x % 2] += x;
    }

    if (sum[0] > sum[1])
        return "even";
    else if (sum[0] < sum[1])
        return "odd";
    else
        return "equals";
}

int
main()
{
    string filename;
    cin >> filename;
    cout << func(filename) << endl;
    return 0;
}
