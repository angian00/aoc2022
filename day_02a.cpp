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
        string move_my = line.substr(delim_pos+1, line.length()-delim_pos);

        //cout << move_opponent << " vs " << move_my << "\n";

        int score_round = 0;

        if (move_my == "X") {
            score_round += 1;

            if (move_opponent == "A")
                score_round += 3;
            else if (move_opponent == "B")
                score_round += 0;
            else
                score_round += 6;

        } else if (move_my == "Y") {
            score_round += 2;

            if (move_opponent == "A")
                score_round += 6;
            else if (move_opponent == "B")
                score_round += 3;
            else
                score_round += 0;

        } else if (move_my == "Z") {
            score_round += 3;

            if (move_opponent == "A")
                score_round += 0;
            else if (move_opponent == "B")
                score_round += 6;
            else
                score_round += 3;
        }

        score_total += score_round;
    }

    cout << score_total << "\n";

    return 0;
}
