#include <iostream>
#include <fstream>
#include <string>

using namespace std;


const int sampled_cycles[] = {
    20,
    60,
    100,
    140,
    180,
    220
};

const int n_sampled_cycles = sizeof(sampled_cycles)/sizeof(int);



int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    int total = 0;

    int x_before = 1;
    int x_after;
    int cycle_before = 1;
    int cycle_after;
    int i_sampled_cycle = 0;
    int tot_signal_strength = 0;

    while (getline(f, line))
    {
        if (line.substr(0, 4) == "noop") {
            cout << "noop" << endl;
            cycle_after = cycle_before + 1;
            x_after = x_before;
        
        } else {
            int delta = stoi(line.substr(5, line.length() - 5));
            cout << "addx " << delta << endl;
            x_after = x_before + delta;
            cycle_after = cycle_before + 2;
        }

        if (cycle_after > sampled_cycles[i_sampled_cycle]) {
            cout << "cycle_before: " << cycle_before << " cycle_after: " << cycle_after <<
                    "  x_before: " << x_before << " signal strength: " << sampled_cycles[i_sampled_cycle] * x_before << endl;
            tot_signal_strength += sampled_cycles[i_sampled_cycle] * x_before;
            i_sampled_cycle ++;

            if (i_sampled_cycle >= n_sampled_cycles)
                break;
        }

        //cout << "cycle_before: " << cycle_before << " cycle_after: " << cycle_after 
        //       << "  x_before=" << x_before << " x_after=" << x_after << endl;

        cycle_before = cycle_after;
        x_before = x_after;
    }

    cout << tot_signal_strength << "\n";

    return 0;
}
