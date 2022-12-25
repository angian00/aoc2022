#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
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
                    //TODO: push_back tile
                } else {
                    Direction dir = char2dir(c);
                    blizzards.push_back(new Blizzard(parsing_pos, dir));
                }

                parsing_pos.x ++;
            }

            parsing_pos.y ++;
        }

        void update_bounds() {
            w = parsing_pos.x;
            h = parsing_pos.y;
        }


        bool is_wall(GridPos pos) {
            if (pos.x == 0)
                return true;

            if (pos.x == w-1)
                return true;

            if (pos.y < 0) {
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
            vector<Blizzard*> to_be_removed;

            for (auto b: blizzards) {
                GridPos next_pos = apply_dir(b->pos, b->dir);

                if (next_pos.y == 0) {
                    if (is_wall(next_pos))
                        next_pos.y = h-2;
                    else
                        to_be_removed.push_back(b);
                }

                if (next_pos.x == 0)
                    next_pos.x = w-2;

                if (next_pos.y == h-1)
                    if (is_wall(next_pos))
                        next_pos.y = 1;
                    else
                        to_be_removed.push_back(b);

                if (next_pos.x == w-1)
                    next_pos.x = 1;

                b->pos = next_pos;
            }

            for (auto b: to_be_removed) {
                //cout << "Removing blizzard " << endl;
                auto index = std::find(blizzards.begin(), blizzards.end(), b);
                blizzards.erase(index);
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


    private:

        vector<Blizzard*> blizzards;
        //vector<vector<bool>> tiles;
        vector<GridPos> path;
        GridPos parsing_pos = GridPos(0, 0);

        GridPos expedition_pos;

        int w;
        int h;
};



/*
const int max_rounds = 25;
const int w = 7;
const int h = 7;
*/

/*
const int max_rounds = 25;
const int w = 8;
const int h = 6;
*/

const int max_rounds = 250;
//const int max_rounds = 450;
const int w = 122;
const int h = 27;


class Optimizer {
    public:
        Optimizer(GridMap grid_map) {
            cout << "Optimizer constructor" << endl;

            for (int i_round=0; i_round < max_rounds; i_round++) {
                if (i_round % 100 == 0)
                    cout << "i_round=" << i_round << endl;

                for (int y=0; y < h; y++) {
                    for (int x=0; x < w; x++) {
                        GridPos pos(x, y);
                        bool is_tile_free = true;

                        if (grid_map.is_wall(pos))
                            is_tile_free = false;
                        
                        if (grid_map.get_blizzards(pos).size() > 0)
                            is_tile_free = false;

                        grid_state[i_round][y][x] = is_tile_free;
                    }
                }

                grid_map.move_blizzards();
            }

        }

        void compute_path() {
            cout << "Optimizer compute_path" << endl;

            auto start_path = new vector<GridPos>();

            start_path->push_back(start_pos);
            compute_path_recurse(max_rounds, start_path, &path);
        }

        int compute_path_len() {
            cout << "compute_path_len" << endl;

            return compute_path_recurse(max_rounds, 0, start_pos);
        }

        vector<GridPos>* get_path() {
            return path;
        }


    private:
        int compute_path_recurse(int best_sol_so_far, int i_round, GridPos curr_pos) {
            if (i_round < 50)
                cout << "compute_path_recurse; i_round=" << i_round << ", curr_pos=" << curr_pos << endl;
            

            if (curr_pos == end_pos) {
                return i_round + 1;
            }

            Direction dir;
            for (int i_dir=0; i_dir < 5; i_dir++) {
                GridPos next_pos;
                if (i_dir < 4) {
                    dir = prioritized_dirs[i_dir];
                    //cout << "examining dir: " << dir << endl;
                    next_pos = apply_dir(curr_pos, dir);

                } else {
                    //cout << "waiting still" << endl;
                    next_pos = curr_pos;
                }
                
                if (is_valid_pos(next_pos, i_round)) {
                    //cout << "i_round = " << i_round << " next_pos " << next_pos << " is valid" << endl;

                    if (best_sol_so_far <= lower_bound(next_pos, i_round))
                        continue;

                    //cout << "lower_bound is passed" << endl;
                    
                    //cout << "i_round = " << i_round << " recursing" << endl;
                    int new_sol = compute_path_recurse(best_sol_so_far, i_round+1, next_pos);

                    if (new_sol < best_sol_so_far) {
                        //cout << "i_round = " << i_round << " found new best: " << new_sol << " p_tmp_final_path=" << p_tmp_final_path << endl;
                        best_sol_so_far = new_sol;

                    }
                }
            }

            return best_sol_so_far;
        }


        int compute_path_recurse(int best_sol_so_far, vector<GridPos>* path_so_far, vector<GridPos>** final_path) {
            GridPos curr_pos = path_so_far->at(path_so_far->size()-1);
            int i_round = path_so_far->size();
            
            if (i_round < 50)
                cout << "compute_path_recurse; i_round=" << i_round << ", curr_pos=" << curr_pos << endl;
            

            vector<GridPos>* p_new_final_path = nullptr;
            
            if (curr_pos == end_pos) {
                (*final_path) = new vector<GridPos>(*path_so_far);
                //cout << "compute_path_recurse; final step; allocating (*final_path): " << (*final_path) << endl;
                //cout << "compute_path_recurse; final step; *final_path->size: " << (*final_path)->size() << endl;

                return (*final_path)->size();
            }

            Direction dir;
            for (int i_dir=0; i_dir < 5; i_dir++) {
                GridPos next_pos;
                if (i_dir < 4) {
                    dir = prioritized_dirs[i_dir];
                    //cout << "examining dir: " << dir << endl;
                    next_pos = apply_dir(curr_pos, dir);

                } else {
                    //cout << "waiting still" << endl;
                    next_pos = curr_pos;
                }
                
                if (is_valid_pos(next_pos, i_round)) {
                    //cout << "i_round = " << i_round << " next_pos " << next_pos << " is valid" << endl;

                    if (best_sol_so_far <= lower_bound(next_pos, i_round))
                        continue;

                    //cout << "lower_bound is passed" << endl;
                    
                    auto new_path = new vector<GridPos>(*path_so_far);
                    new_path->push_back(next_pos);

                    vector<GridPos>* p_tmp_final_path = nullptr;
                    //cout << "i_round = " << i_round << " recursing" << endl;
                    int new_sol = compute_path_recurse(best_sol_so_far, new_path, &p_tmp_final_path);

                    if (new_sol < best_sol_so_far) {
                        //cout << "i_round = " << i_round << " found new best: " << new_sol << " p_tmp_final_path=" << p_tmp_final_path << endl;
                        
                        p_new_final_path = p_tmp_final_path;
                        best_sol_so_far = new_sol;

                    } else {
                        //cout << "i_round = " << i_round << " freeing " << p_tmp_final_path << endl;
                        if (p_tmp_final_path != nullptr) {
                            free(p_tmp_final_path);
                        }
                    }
                }
            }

            if (p_new_final_path != nullptr) {
                //cout << "i_round = " << i_round << " returning new best: " << best_sol_so_far << endl;

                *final_path = p_new_final_path;
            } else  {
                *final_path = nullptr;
            }

            return best_sol_so_far;
        }


        int lower_bound(GridPos curr_pos, int i_round) {
            return i_round + abs(curr_pos.x - end_pos.x) + abs(curr_pos.y - end_pos.y);
        }


        bool is_valid_pos(GridPos pos, int i_round) {
            return (i_round < max_rounds && pos.x >= 0 && pos.x < w && pos.y >= 0 && pos.y < h && grid_state[i_round][pos.y][pos.x]);
        }

        GridPos start_pos = GridPos(1, 0);
        GridPos end_pos = GridPos(w-2, h-1);
        vector<GridPos>* path;
        bool grid_state[max_rounds][h][w];
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
    grid_map.update_bounds();
    //grid_map.print();

    clock_t t_start = clock();
    Optimizer optimizer(grid_map);
    //optimizer.compute_path();

    // auto path = optimizer.get_path();
    // for (auto pos: *path) {
    //     cout << "\t" << pos << endl;
    // }

//    int total_len = path->size() - 1;
    int total_len = optimizer.compute_path_len();

    
    clock_t t_end = clock();
    cout << "result: " << total_len << endl;
    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
