#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


const int n_knots = 9;
//const int n_knots = 1;

struct GridPos {
    int x;
    int y;
};


class Rope
{
    public:
        Rope(int size, int start_x, int start_y): size(size), visited(size)
        {
            head_pos.x = start_x;
            head_pos.y = start_y;
            for (int i_knot=0; i_knot < n_knots; i_knot++) {
                knots_pos[i_knot].x = start_x;
                knots_pos[i_knot].y = start_y;
            }

            for (int x=0; x < size; x++) {
                visited[x] = vector<bool>(size);
                for (int y=0; y < size; y++) {
                    visited[x][y] = false;
                }
            }

            auto tail_pos = knots_pos[n_knots-1];
            visited[tail_pos.x][tail_pos.y] = true;
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
            //cout << "move_head " << dir << endl;

            switch (dir) {
                case 'U':
                    head_pos.y ++;
                    break;

                case 'D':
                    head_pos.y --;
                    break;

                case 'R':
                    head_pos.x ++;
                    break;

                case 'L':
                    head_pos.x --;
                    break;
            }

            GridPos* last_knot = &head_pos;
            GridPos* next_knot = &knots_pos[0];
            
            for (int i_knot=0; i_knot < n_knots; i_knot++) {
                update_knot(last_knot, next_knot);
                last_knot = next_knot;
                next_knot ++;
            }

            auto tail_pos = knots_pos[n_knots-1];

            visited[tail_pos.x][tail_pos.y] = true;
        }

        void update_knot(GridPos* knot_ahead, GridPos* knot_behind)
        {
            if ((knot_ahead->x == knot_behind->x) && (abs(knot_ahead->y - knot_behind->y) == 2)) {
                //cout << "straight move y" << endl;
                knot_behind->y += (knot_ahead->y - knot_behind->y) / 2;
            } else if ((knot_ahead->y == knot_behind->y) && (abs(knot_ahead->x - knot_behind->x) == 2)) {
                //cout << "straight move x" << endl;
                knot_behind->x += (knot_ahead->x - knot_behind->x) / 2;
            }

            //diagonal move
            else if (abs(knot_ahead->x - knot_behind->x) + abs(knot_ahead->y - knot_behind->y) > 2) {
                //cout << "diagonal move" << endl;
                if ((knot_ahead->x - knot_behind->x) > 0)
                    knot_behind->x ++;
                else
                    knot_behind->x --;

                if ((knot_ahead->y - knot_behind->y) > 0)
                    knot_behind->y ++;
                else
                    knot_behind->y --;
            }
        }


        void print() {
            for (int y=size-1; y >= 0; y--) {
                for (int x=0; x < size; x++) {
                    char c;

                    if (head_pos.x == x && head_pos.y == y) {
                        c = 'H';
                    } else {
                        bool found = false;
                        for (int i_knot=0; i_knot < n_knots; i_knot++) {
                            if (knots_pos[i_knot].x == x && knots_pos[i_knot].y == y) {
                                c = '0' + (i_knot+1);
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                            c = '.';
                    }
                    
                    cout << c;
                }

                cout << endl;
            }
            
            cout << endl;
        }


    private:
        int size;
        
        GridPos head_pos;
        GridPos knots_pos[n_knots];

        vector<vector<bool>> visited;
};

int main(int argc, char *argv[])
{
    string filename = argv[1];

    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    
    //Rope rope(27, 12, 5);
    Rope rope(800, 250, 250);
    //Rope rope(20, 10, 10);

    int i_lines = 0;
    while (getline(f, line))
    {
        char dir = line[0];
        int n_moves = stoi(line.substr(2, line.length()-2));

        for (int i=0; i < n_moves; i++) {
            rope.move_head(dir);
        }

        i_lines ++;
    }

    //rope.print();

    cout << rope.count_visited() << "\n";

    return 0;
}
