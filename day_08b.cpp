#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


int compute_scenic_score(vector<vector<int>> heights, int start_row, int start_col)
{
    int n_rows = heights.size();
    int n_cols = heights[0].size();

    //cout << "compute_scenic_score(" << start_row << ", " << start_col << ")" << endl;

    int start_height = heights[start_row][start_col];
    int total_score = 1;
    int curr_dir_score;

    curr_dir_score = 0;
    for (int col=start_col+1; col < n_cols; col++) {
        curr_dir_score ++;
        if (heights[start_row][col] >= start_height) {
            break;
        }
    }
    total_score *= curr_dir_score;
    //cout << curr_dir_score << " " << total_score << endl;

    curr_dir_score = 0;
    for (int col=start_col-1; col >=0; col--) {
        curr_dir_score ++;
        if (heights[start_row][col] >= start_height) {
            break;
        }
    }
    total_score *= curr_dir_score;
    //cout << curr_dir_score << " " << total_score << endl;

    curr_dir_score = 0;
    for (int row=start_row+1; row < n_rows; row++) {
        curr_dir_score ++;
        if (heights[row][start_col] >= start_height) {
            break;
        }
    }
    total_score *= curr_dir_score;
    //cout << curr_dir_score << " " << total_score << endl;

    curr_dir_score = 0;
    for (int row=start_row-1; row >=0; row--) {
        curr_dir_score ++;
        if (heights[row][start_col] >= start_height) {
            break;
        }
    }
    total_score *= curr_dir_score;
    //cout << curr_dir_score << " " << total_score << endl;

    return total_score;
}


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

    int max_score = 0;
    for (int row=0; row < n_rows; row++) {
        for (int col=0; col < n_cols; col++) {
            int curr_score = compute_scenic_score(heights, row, col);
            if (curr_score > max_score)
                max_score = curr_score;
        }
    }

    cout << max_score << "\n";

    return 0;
}
