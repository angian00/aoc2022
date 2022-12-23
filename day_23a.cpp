#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <ctime>
#include <cassert>


using namespace std;


struct GridPos {
    int x;
    int y;

    GridPos(int x, int y): x(x), y(y) {}
    GridPos(): x(-1), y(-1) {}

    friend bool operator==(const GridPos& lhs, const GridPos& rhs) { 
        return ( (lhs.x == rhs.x) && (lhs.y == rhs.y) );
    }

    friend bool operator<(const GridPos& lhs, const GridPos& rhs)
    {
      return (lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y) );
    }

    friend std::ostream & operator<<( std::ostream &os, const GridPos &value )
    {
        os << "(" << value.x << ", " << value.y << ")";
        
        return os;
    }

};


enum Direction {
    North,
    South,
    West,
    East
};

Direction increment_direction(Direction dir) {
    int next_dir_int = dir + 1;
    if (next_dir_int == 4)
        next_dir_int = 0;
    
    return (Direction) next_dir_int;
}

class Elf {
    public:
        Elf(GridPos start_pos) {
            pos = start_pos;
            next_dir = Direction::North;
        }
        
        GridPos pos;
        GridPos chosen_pos;
        Direction next_dir;
};


class GridMap {
    public:
        void parse_row(string s) {
            //cout << "parse row" << endl;

            parsing_pos.x = 0;

            for (auto c: s) {
                if (c == '#')
                    elves.push_back(Elf(parsing_pos));

                parsing_pos.x ++;
            }

            parsing_pos.y ++;
        }

        void update_tiles() {
            update_bounds();

            tiles.clear();

            for (int y=y_min; y <= y_max; y++) {
                tiles.push_back(vector<bool>());

                for (int x=x_min; x <= x_max; x++) {
                    tiles[y-y_min].push_back(false);
                }
            }

            for (auto elf: elves) {
                tiles[elf.pos.y-y_min][elf.pos.x-x_min] = true;
            }
        }

        void compute_rounds(int n_rounds) {
            for (int i=0; i < n_rounds; i++) {
                compute_round();
                //print();
                //break; //DEBUG
            }
        }


        int get_n_empty_tiles() {
            int n_empty = 0;

            for (int y=y_min; y <= y_max; y++) {
                for (int x=x_min; x <= x_max; x++) {
                    if (is_tile_empty(x, y))
                        n_empty++;
                }
            }

            return n_empty;
        }


        void print() {
            cout << "x from " << x_min << " to " << x_max << endl;
            cout << "y from " << y_min << " to " << y_max << endl;

            for (int y=y_min; y <= y_max; y++) {
                for (int x=x_min; x <= x_max; x++) {
                    cout << (is_tile_empty(x, y) ? "." : "#");
                }
                
                cout << endl;
            }

            cout << endl;
        }


    private:
        void compute_round() {
            cout << "compute round" << endl;

            for (int i_elf=0; i_elf < elves.size(); i_elf++) {
                choose_next_pos(&(elves[i_elf]));
            }


            set<GridPos> chosen_dests;
            for (int i_elf=0; i_elf < elves.size(); i_elf++) {
                //cout << "compute_round; i_elf =" << i_elf << "/" << elves.size() << endl;

                auto curr_elf = elves[i_elf];
                bool is_dest_conflicting = false;

                if (curr_elf.chosen_pos.x == INT_MIN) {
                    //cout << "no valid direction " << endl;
                    continue;
                }

                if (chosen_dests.contains(curr_elf.chosen_pos)) {
                    //cout << "is_dest_conflicting (a)" << endl;

                    is_dest_conflicting = true;
                } else {
                    for (int j_elf=i_elf+1; j_elf < elves.size(); j_elf++) {
                        if (elves[j_elf].chosen_pos == curr_elf.chosen_pos) {
                            //cout << "is_dest_conflicting (b)" << endl;

                            is_dest_conflicting = true;
                            break;
                        }
                    }
                }

                chosen_dests.insert(curr_elf.chosen_pos);

                if (!is_dest_conflicting) {
                    //cout << "moving elf from " << curr_elf.pos << " to " << curr_elf.chosen_pos << endl;
                    elves[i_elf].pos = elves[i_elf].chosen_pos;
                }
            }

            for (int i_elf=0; i_elf < elves.size(); i_elf++) {
                elves[i_elf].next_dir = increment_direction(elves[i_elf].next_dir);
            }

            update_tiles();
        }

        
        void choose_next_pos(Elf* elf) {
            int n_computed_dirs = 0;
            int n_free_dirs = 0;
            auto dir = elf->next_dir;

            GridPos chosen_pos;
            
            //cout << "choose_next_pos; start pos=" << elf->pos << " start chosen_pos=" << elf->chosen_pos << endl;

            while (n_computed_dirs < 4) {
                GridPos next_pos;
                GridPos pos1, pos2;

                switch (dir) {
                    case Direction::North:
                        next_pos.x = elf->pos.x;
                        next_pos.y = elf->pos.y - 1;

                        pos1.x = elf->pos.x - 1;
                        pos1.y = elf->pos.y - 1;
                        pos2.x = elf->pos.x + 1;
                        pos2.y = elf->pos.y - 1;
                        break;


                    case Direction::South:
                        next_pos.x = elf->pos.x;
                        next_pos.y = elf->pos.y + 1;

                        pos1.x = elf->pos.x - 1;
                        pos1.y = elf->pos.y + 1;
                        pos2.x = elf->pos.x + 1;
                        pos2.y = elf->pos.y + 1;
                        break;

                    case Direction::West:
                        next_pos.x = elf->pos.x - 1;
                        next_pos.y = elf->pos.y;

                        pos1.x = elf->pos.x - 1;
                        pos1.y = elf->pos.y - 1;
                        pos2.x = elf->pos.x - 1;
                        pos2.y = elf->pos.y + 1;
                        break;

                    case Direction::East:
                        next_pos.x = elf->pos.x + 1;
                        next_pos.y = elf->pos.y;

                        pos1.x = elf->pos.x + 1;
                        pos1.y = elf->pos.y - 1;
                        pos2.x = elf->pos.x + 1;
                        pos2.y = elf->pos.y + 1;
                        break;
                }
                
                if (is_tile_empty(next_pos) && is_tile_empty(pos1) && is_tile_empty(pos2)) {
                    //cout << "updating chosen pos for elf to " << next_pos << endl;
                    if (n_free_dirs == 0)
                        chosen_pos = next_pos;

                    n_free_dirs ++;
                }

                n_computed_dirs ++;
                dir = increment_direction(dir);
            }

            if (n_free_dirs > 0 && n_free_dirs < 4)
                elf->chosen_pos = chosen_pos;
            else
                elf->chosen_pos = GridPos(INT_MIN, INT_MIN);

            //cout << "choose_next_pos; end chosen_pos=" << elf->chosen_pos << endl;
        }


        void update_bounds() {
            x_min = INT_MAX;
            x_max = INT_MIN;
            y_min = INT_MAX;
            y_max = INT_MIN;

            for (auto elf: elves) {
                if (elf.pos.x < x_min)
                    x_min = elf.pos.x;
                if (elf.pos.x > x_max)
                    x_max = elf.pos.x;

                if (elf.pos.y < y_min)
                    y_min = elf.pos.y;
                if (elf.pos.y > y_max)
                    y_max = elf.pos.y;
            }
        }

        bool is_tile_empty(GridPos pos) {
            return is_tile_empty(pos.x, pos.y);
        }

        bool is_tile_empty(int x, int y) {
            if (x < x_min || x > x_max || y < y_min || y > y_max)
                return true;

            return (!tiles[y-y_min][x-x_min]);
        }

        vector<Elf> elves;
        vector<vector<bool>> tiles;
        int x_min;
        int x_max;
        int y_min;
        int y_max;

        GridPos parsing_pos = GridPos(0, 0);
};



int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);


    GridMap grid_map;

    while (getline(f, line))
    {
        grid_map.parse_row(line);
    }

    clock_t t_start = clock();
    grid_map.update_tiles();
    grid_map.print();

    grid_map.compute_rounds(10);
    int n_empty = grid_map.get_n_empty_tiles();

    clock_t t_end = clock();
    cout << "result: " << n_empty << endl;
    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
