#include <iostream>
#include <fstream>
#include <string>

using namespace std;


const int marker_len = 14;


bool is_valid_marker(string token)
{
    for (int i=1; i < token.length(); i++)
    {
        for (int j=0; j < i; j++)
        {
            //cout << i << ", " << j << ": [" << token[i] << "] [" << token[j] << "]" << endl;
            if (token[i] == token[j])
                return false;
        }
    }

    return true;
}


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    getline(f, line);

    int pos = 0;
    while (pos < line.length() - marker_len) {
        string curr_token = line.substr(pos, marker_len);
        if (is_valid_marker(curr_token)) {
            break;
        }


        pos ++;
    }

    pos += marker_len;

    cout << pos << "\n";

    return 0;
}
