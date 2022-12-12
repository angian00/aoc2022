#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    int curr_total = 0;
    vector<int> max_totals;
    for (int i = 0; i < 3; i++)
        max_totals.push_back(0);

    while (getline(f, line))
    {
        if (line.length() == 0)
        {
            vector<int>::iterator it = max_totals.begin();
            while (it != max_totals.end())
            {
                if (curr_total > *it)
                {
                    max_totals.insert(it, curr_total);
                    max_totals.pop_back();
                    break;
                }

                it ++;
            }

            curr_total = 0;
            continue;
        }

        int curr_calories =  std::stoi(line);
        //cout << "Found val " << curr_calories << "\n";
        curr_total += curr_calories;
    }

    int tot_total = 0;
    for (vector<int>::iterator it = max_totals.begin() ; it != max_totals.end(); ++it)
        tot_total += *it;

    cout << tot_total << "\n";

    return 0;
}
