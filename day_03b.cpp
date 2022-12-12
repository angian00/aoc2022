#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ranges>


using namespace std;


int get_priority(const char);


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    int total = 0;

    string rucksack1;
    string rucksack2;
    string rucksack3;

    while (true)
    {
        if (!getline(f, rucksack1))
            break;
        
        getline(f, rucksack2);
        getline(f, rucksack3);

        vector<char> common_chars;

        for (auto c: rucksack1)
        {
            if (rucksack2.find(c) != string::npos)
                common_chars.push_back(c);
        }

        for (auto c: common_chars)
        {
            if (rucksack3.find(c) != string::npos) {
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