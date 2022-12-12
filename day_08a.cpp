#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


int main(int argc, char *argv[])
{
    string filename = argv[1];
    vector<vector<int>> heights;
    vector<vector<bool>> visibles;

    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);


    int row = 0;
    while (getline(f, line))
    {
        heights.push_back(vector<int>(line.length()));
        visibles.push_back(vector<bool>(line.length()));

        for (int col=0; col < line.length(); col++) {
            heights[row][col] = line[col] - '0';
            visibles[row][col] = false;
        }

        row ++;
    }


    int n_rows = heights.size();
    int n_cols = heights[0].size();

    for (int row=0; row < n_rows; row++) {
        int max_so_far = -1;
        for (int col=0; col < n_cols; col++) {
            if (heights[row][col] > max_so_far) {
                max_so_far = heights[row][col];
                visibles[row][col] = true;
            }
        }
    }

    for (int row=0; row < n_rows; row++) {
        int max_so_far = -1;
        for (int col = n_cols-1; col >= 0; col--) {
            if (heights[row][col] > max_so_far) {
                max_so_far = heights[row][col];
                visibles[row][col] = true;
            }
        }
    }
    
    for (int col=0; col < n_cols; col++) {
        int max_so_far = -1;
        for (int row=0; row < n_rows; row++) {
            if (heights[row][col] > max_so_far) {
                max_so_far = heights[row][col];
                visibles[row][col] = true;
            }
        }
    }

    for (int col=0; col < n_cols; col++) {
        int max_so_far = -1;
        for (int row = n_cols-1; row >=0; row--) {
            if (heights[row][col] > max_so_far) {
                max_so_far = heights[row][col];
                visibles[row][col] = true;
            }
        }
    }

    int n_visible = 0;
    for (int row=0; row < n_rows; row++)
        for (int col=0; col < n_cols; col++)
            if (visibles[row][col])
                n_visible ++;
    
    cout << n_visible << "\n";

    return 0;
}
