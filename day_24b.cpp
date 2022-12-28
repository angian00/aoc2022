#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <algorithm>
#include <ctime>
#include <cassert>


using namespace std;




// const int w = 8;
// const int h = 6;

const int w = 122;
const int h = 27;


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
    East,
    South,
    North,
    West,
};

const Direction prioritized_dirs[] = {
    Direction::East,
    Direction::South,
    Direction::North,
    Direction::West,
};

GridPos apply_dir(GridPos from_pos, Direction dir) {
    GridPos to_pos(from_pos);

    switch (dir) {
        case Direction::North:
            to_pos.y --;
            break;
        case Direction::East:
            to_pos.x ++;
            break;
        case Direction::South:
            to_pos.y ++;
            break;
        case Direction::West:
            to_pos.x --;
            break;
    }

    return to_pos;
}


Direction char2dir(char c) {
    switch (c) {
        case '^':
            return Direction::North;
        case '>':
            return Direction::East;
        case 'v':
            return Direction::South;
        case '<':
            return Direction::West;
        default:
            assert(false);
            return Direction::North;
    }
}

char dir2char(Direction dir) {
    switch (dir) {
        case Direction::North:
            return '^';
        case Direction::East:
            return '>';
        case Direction::South:
            return 'v';
        case Direction::West:
            return '<';
        default:
            return '?';
    }
}

string dir2str(int i_dir) {
    switch (i_dir) {
        case Direction::North:
            return "north";
        case Direction::East:
            return "east";
        case Direction::South:
            return "south";
        case Direction::West:
            return "west";
        case 4:
            return "wait";
        default:
            return "???";
    }
}

std::ostream & operator<<( std::ostream &os, const Direction &dir )
{
    os << dir2char(dir);
    
    return os;
}


class Blizzard {
    public:
        Blizzard(GridPos pos, Direction dir): pos(pos), dir(dir) {}
        
        GridPos pos;
        Direction dir;
};


class GridMap {
    public:
        void parse_row(string s) {
            parsing_pos.x = 0;

            for (auto c: s) {
                if (c == '.' || c == '#') {
                    //ignore
                } else {
                    Direction dir = char2dir(c);
                    blizzards.push_back(new Blizzard(parsing_pos, dir));
                }

                parsing_pos.x ++;
            }

            parsing_pos.y ++;
        }

        bool is_wall(GridPos pos) {
            if (pos.x <= 0)
                return true;

            if (pos.x >= w-1)
                return true;

            if (pos.y < 0) {
                return true;
            }

            if (pos.y >= h) {
                return true;
            }

            if (pos.y == 0) {
                if (pos.x == 1)
                    return false;
                else
                    return true;
            }

            if (pos.y == h-1) {
                if (pos.x == w-2)
                    return false;
                else
                    return true;
            }

            return false;
        }


        void print() {
            for (int y=0; y < h; y++) {
                for (int x=0; x < w; x++) {
                    GridPos pos(x, y);
                    char c;

                    if (is_wall(pos))
                        c = '#';
                    else if (pos == expedition_pos)
                        c = 'E';
                    else {
                        auto tile_blizzards = get_blizzards(pos);
                        if (tile_blizzards.size() == 0)
                            c = '.';
                        else if (tile_blizzards.size() == 1)
                            c = dir2char(tile_blizzards[0]->dir);
                        else
                            c = '0' + tile_blizzards.size();
                    }

                    cout << c;
                }
                
                cout << endl;
            }

            cout << endl;
        }

        void move_blizzards() {
            //we know that in input there are no blizzards going through start or end pos
            for (auto b: blizzards) {
                GridPos next_pos = apply_dir(b->pos, b->dir);

                if (next_pos.y == 0) {
                    next_pos.y = h-2;
                }

                if (next_pos.x == 0)
                    next_pos.x = w-2;

                if (next_pos.y == h-1)
                    next_pos.y = 1;

                if (next_pos.x == w-1)
                    next_pos.x = 1;

                b->pos = next_pos;
            }
        }

        vector<Blizzard*> get_blizzards(GridPos pos) {
            vector<Blizzard*> res;
            for (auto b: blizzards) {
                if (b->pos == pos)
                    res.push_back(b);
            }

            return res;
        }


        bool is_blizzard_free(GridPos pos) {
            for (auto b: blizzards) {
                if (b->pos == pos)
                    return false;
            }

            return true;
        }


    private:

        vector<Blizzard*> blizzards;
        vector<GridPos> path;
        GridPos parsing_pos = GridPos(0, 0);

        GridPos expedition_pos;
};



class Optimizer {
    public:
        Optimizer(GridMap grid_map): grid_map(grid_map)  {}

        void compute_path_len() {
            cout << "compute_path_len" << endl;

            int trip1 = compute_path(start_pos, end_pos);
            cout << "first trip: " << trip1 << endl;
            int trip2 = compute_path(end_pos, start_pos);
            cout << "second trip: " << trip2 << endl;
            int trip3 = compute_path(start_pos, end_pos);
            cout << "third trip: " << trip3 << endl;

            cout << "total: " << (trip1 + trip2 + trip3) << endl;
            cout << "n_explored_nodes: " << n_explored_nodes << endl;
            cout << "n_leaves: " << n_leaves << endl;
            cout << "n_lower_bound: " << n_lower_bound << endl;
        }


    private:
        int compute_path(GridPos from_pos, GridPos to_pos) {
            int i_round = 0;
            
            set<GridPos>* curr_round_pos;
            set<GridPos>* next_round_pos = new set<GridPos>();
            next_round_pos->insert(from_pos);

            while (true) {
                curr_round_pos = next_round_pos;
                next_round_pos = new set<GridPos>();
                //cout << "computing round #" << i_round << "; # curr_round_pos=" << curr_round_pos->size() << endl;

                grid_map.move_blizzards();

                for (auto curr_pos: (*curr_round_pos)) {
                    // if (from_pos.y != 0)
                    //     cout << "curr_pos: " << curr_pos << endl;
                    for (auto next_pos: enumerate_steps(curr_pos)) {
                        if (next_pos == to_pos) {
                            //found solution
                            return i_round + 1;
                        }
                
                        next_round_pos->insert(next_pos);
                    }
                }

                i_round ++;
            }
        }

        vector<GridPos> enumerate_steps(GridPos curr_pos) {
            vector<GridPos> res;

            for (int i_dir=0; i_dir < 5; i_dir++) {
                GridPos next_pos;
                Direction dir;
                if (i_dir < 4) {
                    dir = prioritized_dirs[i_dir];
                    //cout << "examining dir: " << dir << endl;
                    next_pos = apply_dir(curr_pos, dir);

                } else {
                    //cout << "waiting still" << endl;
                    next_pos = curr_pos;
                }

                if (!grid_map.is_wall(next_pos) && (grid_map.is_blizzard_free(next_pos)))
                    res.push_back(next_pos);
            }

            return res;
        }


        GridPos start_pos = GridPos(1, 0);
        GridPos end_pos = GridPos(w-2, h-1);
        GridMap grid_map;

        unsigned long long n_explored_nodes = 0;
        unsigned long long n_leaves = 0;
        unsigned long long n_lower_bound = 0;
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
    //grid_map.print();

    clock_t t_start = clock();

    Optimizer optimizer(grid_map);
    optimizer.compute_path_len();
    
    clock_t t_end = clock();

    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
