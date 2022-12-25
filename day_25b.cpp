#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <cassert>

using namespace std;


long long snafu2int(string s) {
    long long val = 0;

    long long factor = 1;
    for (int pos=0; pos < s.length(); pos++) {
        char digit = s[s.length() - pos - 1];
        long long digit_value = 0;
        switch (digit) {
            case '1':
                digit_value = 1;
                break;
            case '2':
                digit_value = 2;
                break;
            case '-':
                digit_value = -1;
                break;
            case '=':
                digit_value = -2;
                break;
        }
        
        val += factor * digit_value;
        factor *= 5;
    }

    return val;
}


string int2snafu(long long val) {
    vector<char> chars;

    long long curr_val = val;
    long long factor = 5;
    long long last_factor = 1;
    long long rem;

    int pos = 0;
    while (true) {
        rem = (curr_val % factor) / last_factor;
        cout << "curr_val: " << curr_val << " factor: " << factor << " rem: " << rem << endl;
        assert(rem >= 0 && rem < 5);

        char c;
        bool plus_one = false;
        if (rem == 0)
            c = '0';
        else if (rem == 1)
            c = '1';
        else if (rem == 2)
            c = '2';
        else if (rem == 3) {
            c = '=';
            plus_one = true;
        } else {
            c = '-';
            plus_one = true;
        }

        chars.push_back(c);

        curr_val -= rem * last_factor;
        if (plus_one)
            curr_val += factor;

        if (curr_val == 0)
            break;

        last_factor = factor;
        factor *= 5;
        pos ++;
    }

    stringstream ss;
    for (int pos=chars.size()-1; pos >= 0; pos --) {
        ss << chars[pos];
    }

    return ss.str();
}


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);


    long long total = 0;
    while (getline(f, line))
    {
        total += snafu2int(line);
    }

    string tot_str = int2snafu(total);
    cout << "result: " << tot_str << endl;

    return 0;
}
