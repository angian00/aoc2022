#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;




class Rope
{
    public:
        Rope(int size, int start_x, int start_y): size(size), visited(size)
        {
            head_x = start_x;
            head_y = start_y;
            tail_x = start_x;
            tail_y = start_y;
            min_x = 10000;
            max_x = 0;
            min_y = 10000;
            max_y = 0;

            for (int x=0; x < size; x++) {
                visited[x] = vector<bool>(size);
                for (int y=0; y < size; y++) {
                    visited[x][y] = false;
                }
            }

            visited[tail_x][tail_y] = true;
        }

        int count_visited()
        {
            int count = 0;

            for (int x=0; x < size; x++) {
                for (int y=0; y < size; y++) {
                    if (visited[x][y])
                        count ++;
                }
            }

            return count;
        }

        void move_head(char dir)
        {
            //cout << "move_head" << endl;

            switch (dir) {
                case 'U':
                    head_y ++;
                    break;

                case 'D':
                    head_y --;
                    break;

                case 'R':
                    head_x ++;
                    break;

                case 'L':
                    head_x --;
                    break;
            }

            //straight move
            if ((head_x == tail_x) && (abs(head_y - tail_y) == 2))
                tail_y += (head_y - tail_y) / 2;
            else if ((head_y == tail_y) && (abs(head_x - tail_x) == 2))
                tail_x += (head_x - tail_x) / 2;

            //diagonal move
            else if (abs(head_x - tail_x) + abs(head_y - tail_y) > 2) {
                if ((head_x - tail_x) > 0)
                    tail_x ++;
                else
                    tail_x --;

                if ((head_y - tail_y) > 0)
                    tail_y ++;
                else
                    tail_y --;
            }

            //cout << "tail pos:" << tail_x << ", " << tail_y << endl;

            visited[tail_x][tail_y] = true;

            if (tail_x < min_x)
                min_x = tail_x;
            if (tail_x > max_x)
                max_x = tail_x;

            if (tail_y < min_y)
                min_y = tail_y;
            if (tail_y > max_y)
                max_y = tail_y;
        }

        void print() {
            for (int y=size-1; y >= 0; y--) {
                for (int x=0; x < size; x++) {
                    char c;

                    if (head_x == x && head_y == y && tail_x == x && tail_y == y)
                        c = 'X';
                    else if (head_x == x && head_y == y)
                        c = 'H';
                    else if (tail_x == x && tail_y == y)
                        c = 'T';
                    else
                        c = '.';
                    
                    cout << c;
                }

                cout << endl;
            }
            
            cout << endl;
        }


        void print_minmax() {
            cout << "explored grid size - x: " << min_x << " to " << max_x << ", y: " << min_y << " to " << max_y << endl;
        }


    private:
        int size;
        
        int head_x;
        int head_y;
        int tail_x;
        int tail_y;

        int min_x;
        int max_x;
        int min_y;
        int max_y;

        vector<vector<bool>> visited;
};

int main(int argc, char *argv[])
{
    string filename = argv[1];

    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    
    //Rope rope(20, 10, 10);
    Rope rope(800, 250, 250);
    //Rope rope(20, 10, 10);

    int i_lines = 0;
    while (getline(f, line))
    {
        //cout << line << "\n";

        char dir = line[0];
        int n_moves = stoi(line.substr(2, line.length()-2));

        //cout << dir << ", " << n_moves << endl;

        for (int i=0; i < n_moves; i++) {
            rope.move_head(dir);
        }

        //rope.print();
        //cout << rope.count_visited() << "\n";

        //if (i_lines % 2 == 0)
            //rope.print_max();

        //if (i_lines > 10)
            //break;

        i_lines ++;
    }

    cout << rope.count_visited() << "\n";
    rope.print_minmax();

    return 0;
}
