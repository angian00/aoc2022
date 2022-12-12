#include <iostream>
#include <fstream>
#include <string>

using namespace std;

pair<int, int> parse_range(string);


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    int overlap_count = 0;

    while (getline(f, line))
    {
        int delim_pos = line.find(',');
        auto range1 = parse_range(line.substr(0, delim_pos));
        auto range2 = parse_range(line.substr(delim_pos+1, line.length()-delim_pos));
        
        if (range1.first <= range2.second && range1.second >= range2.first)
            overlap_count ++;
    }

    cout << overlap_count << "\n";

    return 0;
}


pair<int, int> parse_range(string range_str)
{
    int delim_pos = range_str.find('-');
    int from = stoi(range_str.substr(0, delim_pos));
    int to = stoi(range_str.substr(delim_pos+1, range_str.length()-delim_pos));

    return pair(from, to);
}