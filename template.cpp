#include <iostream>
#include <fstream>
#include <string>

using namespace std;


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    int total = 0;

    while (getline(f, line))
    {
        //total += curr_value;
    }

    cout << total << "\n";

    return 0;
}
