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

char crt[6][40];


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    int clock = 0;
    char c;
    int x = 1;
    int op_delta = 0;
    int op_duration = 1;

    while (clock < 6*40)
    {
        op_duration --;
        if (op_duration == 0) {
            x += op_delta;

            getline(f, line);
            if (line.substr(0, 4) == "noop") {
                //cout << "noop" << endl;
                op_duration = 1;
                op_delta = 0;
            
            } else {
                op_delta = stoi(line.substr(5, line.length() - 5));
                //cout << "addx " << op_delta << endl;
                op_duration = 2;
            }

        }

        if (abs(clock % 40 - x) <= 1)
            c = '#';
        else
            c = '.';
       
        //cout << "clock " << (clock+1) << " x " << x << endl;

        crt[clock / 40][clock % 40] = c;

        clock ++;
    }

    for (int y=0; y<6; y++) {
        for (int x=0; x<40; x++) {
            cout << crt[y][x];
        }
        cout << endl;
    }

    cout << endl;

    return 0;
}
