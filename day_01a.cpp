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

    int curr_total = 0;
    int max_total = 0;

    while (getline(f, line))
    {
        if (line.length() == 0)
        {
            curr_total = 0;
            continue;
        }

        int curr_calories =  std::stoi(line);
        //cout << "Found val " << curr_calories << "\n";
        curr_total += curr_calories;
        if (curr_total > max_total)
            max_total = curr_total;
    }

    cout << max_total << "\n";

    return 0;
}
