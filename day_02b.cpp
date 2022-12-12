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

    int score_total = 0;

    while (getline(f, line))
    {
        vector<string> tokens;
        int delim_pos = line.find(' ');
        string move_opponent = line.substr(0, delim_pos);
        string result = line.substr(delim_pos+1, line.length()-delim_pos);
        
        string move_my;

        //cout << move_opponent << " vs " << move_my << "\n";

        int score_round = 0;

        if (result == "X") {
            score_round += 0;

            if (move_opponent == "A")
                move_my = "C";
            else if (move_opponent == "B")
                move_my = "A";
            else
                move_my = "B";

        } else if (result == "Y") {
            score_round += 3;

            if (move_opponent == "A")
                move_my = "A";
            else if (move_opponent == "B")
                move_my = "B";
            else
                move_my = "C";

        } else if (result == "Z") {
            score_round += 6;

            if (move_opponent == "A")
                move_my = "B";
            else if (move_opponent == "B")
                move_my = "C";
            else
                move_my = "A";
        }

        if (move_my == "A")
            score_round += 1;
        else if (move_my == "B")
            score_round += 2;
        else
            score_round += 3;

        score_total += score_round;
    }

    cout << score_total << "\n";

    return 0;
}
