#include <iostream>
#include <fstream>
#include <string>

using namespace std;


int get_priority(const char);


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
        size_t len = line.length()/2;

        auto rucksack1 = line.substr(0, len);
        auto rucksack2 = line.substr(len, len);

        //cout << rucksack1 << "   " << rucksack2 << std::endl;
        
        for (auto c: rucksack2)
        {
            if (rucksack1.find(c) != string::npos) {
                total += get_priority(c);
                break;
            }
        }

    }

    cout << total << "\n";

    return 0;
}


int get_priority(const char c)
{
    if ((c >= 'a') && (c <= 'z'))
        return (c - 'a' + 1);
    else
        return (c - 'A' + 27);
}