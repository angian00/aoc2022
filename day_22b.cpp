#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cassert>
#include <ctime>


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

enum Facing {
    Right,
    Down,
    Left,
    Up
};

std::ostream & operator<<( std::ostream &os, const Facing &facing )
{
    string s;
    switch (facing) {
        case Facing::Right:
            s = "right";
            break;
        case Facing::Left:
            s = "left";
            break;
        case Facing::Down:
            s = "down";
            break;
        case Facing::Up:
            s = "up";
            break;
    }

    os << s;
    return os;
}


int find_turn(string s, size_t start_pos) {
    int index = start_pos;
    while (index < s.length()) {
        if (s[index] == 'L' || s[index] == 'R')
            return index;
        
        index ++;
    }

    return -1;
}

enum TileType {
    Wall,
    Open,
    Offset
};


std::ostream & operator<<( std::ostream &os, const TileType &tile_type )
{
    string s;
    switch (tile_type) {
        case TileType::Wall:
            s = "wall";
            break;
        case TileType::Open:
            s = "open";
            break;
        case TileType::Offset:
            s = "offset";
            break;
    }

    os << s;
    return os;
}

//const bool trace = false;
const bool trace = true;

class MonkeyMap {
    public:
        void add_row(string s) {
            vector<TileType> new_row;
            tiles.push_back(new_row);

            int offset_x = 0;
            TileType tile_type; 

            for (auto c: s) {
                if (c == ' ') {
                    offset_x ++;
                    tile_type = TileType::Offset;

                } else if (c == '#') {
                    tile_type = TileType::Wall;
                } else {
                    tile_type = TileType::Open;
                }
                
                tiles[curr_adding_y].push_back(tile_type);
            }

            offsets.push_back(offset_x);
            curr_adding_y ++;
        }


        void set_path(string path) {
            this->path = path;
        }


        void perform_path() {
            curr_facing = Facing::Right;
            curr_pos = GridPos(offsets[0], 0);

            int start_move = 0;
            int turn_index = find_turn(path, start_move);
            while (true) {
                int end_move = ( turn_index == -1 ? path.length() : turn_index );
                int n_steps = stoi(path.substr(start_move, end_move - start_move));
                move(n_steps);

                if (turn_index == -1)
                    break;
                
                turn(path[turn_index] == 'R');

                start_move = turn_index + 1;
                turn_index = find_turn(path, start_move);
            }
        }


        GridPos get_pos() {
            return curr_pos;
        }

        Facing get_facing() {
            return curr_facing;
        }


    private:
        void turn(bool is_right_turn) {
            if (trace) cout << "turning " << (is_right_turn ? "right" : "left") << endl;

            int facing_int;

            if (is_right_turn) {
                facing_int = curr_facing + 1;
                if (facing_int == 4)
                    facing_int = 0;

            } else {
                facing_int = curr_facing - 1;
                if (facing_int == -1)
                    facing_int = 3;
            }

            curr_facing = (Facing)(facing_int);
        }

        void move(int n_steps) {
            if (trace) cout << "moving " << n_steps << " steps " << curr_facing << endl;
            for (int i=0; i < n_steps; i++) {
                bool has_moved = move();
                if (!has_moved)
                    break;
            }
        }

        bool move() {
            if (trace) cout << "trying to move from " << curr_pos << ", facing " << curr_facing << endl;

            GridPos next_pos(curr_pos);
            Facing next_facing = curr_facing;

            switch (curr_facing) {
                case Facing::Right:
                    next_pos.x = curr_pos.x + 1;
                    if (!is_valid_tile(next_pos)) {
                        if (trace) cout << "fold from right" << endl;
                        fold(curr_pos, curr_facing, &next_pos, &next_facing);
                    }
                    break;

                case Facing::Left:
                    next_pos.x = curr_pos.x - 1;
                    if (!is_valid_tile(next_pos)) {
                        if (trace) cout << "fold from left" << endl;
                        fold(curr_pos, curr_facing, &next_pos, &next_facing);
                    }
                    break;
                    
                case Facing::Down:
                    next_pos.y = curr_pos.y + 1;
                    if (!is_valid_tile(next_pos)) {
                        if (trace) cout << "fold from down" << endl;
                        fold(curr_pos, curr_facing, &next_pos, &next_facing);
                    }
                    break;

                case Facing::Up:
                    next_pos.y = curr_pos.y - 1;
                    if (!is_valid_tile(next_pos)) {
                        if (trace) cout << "fold from up" << endl;
                        fold(curr_pos, curr_facing, &next_pos, &next_facing);
                    }
                    break;
            }

            TileType next_tile = tiles[next_pos.y][next_pos.x];
            if (next_tile == TileType::Wall)
                return false;
            
            curr_pos = next_pos;
            curr_facing = next_facing;
            if (trace) cout << "moving to " << curr_pos << " facing " << curr_facing << endl;
            return true;
        }

        bool is_valid_tile(GridPos pos) {
            return is_valid_tile(pos.x, pos.y);
        }

        bool is_valid_tile(int x, int y) {
            if (y < 0)
                return false;
            if (y >= tiles.size())
                return false;

            if (x < offsets[y])
                return false;
            if (x >= tiles[y].size())
                return false;
            
            return true;
        }

        void fold(GridPos start_pos, Facing start_facing, GridPos* p_next_pos, Facing* p_next_facing) {
            //fold_test(start_pos, start_facing, p_next_pos, p_next_facing);
            fold_input(start_pos, start_facing, p_next_pos, p_next_facing);
        }

        void fold_test(GridPos start_pos, Facing start_facing, GridPos* p_next_pos, Facing* p_next_facing) {
            int i_face;
            
            const int cube_size = 4;
            
            int x_in_cube = start_pos.x % cube_size;
            int y_in_cube = start_pos.y % cube_size;

            if (start_pos.y < cube_size) {
                i_face = 1;
            
            } else if (start_pos.y < 2*cube_size) {
                if (start_pos.x < cube_size)
                    i_face = 2;
                else if (start_pos.x < 2*cube_size)
                    i_face = 3;
                else
                    i_face = 4;

            } else {
                if (start_pos.x < 3*cube_size)
                    i_face = 5;
                else
                    i_face = 6;
            }
            
            if (trace) cout << "start_pos: " << start_pos << endl;
            if (trace) cout << "face: " << i_face << " start_facing: " << start_facing <<
                 " x_in_cube: " << x_in_cube << " y_in_cube: " << y_in_cube << endl;

            if (i_face == 1 && start_facing == Facing::Up && y_in_cube == 0) {
                p_next_pos->x = cube_size - 1 - x_in_cube;
                p_next_pos->y = cube_size;
                *p_next_facing = Facing::Down;

            } else if (i_face == 1 && start_facing == Facing::Right && x_in_cube == cube_size - 1) {
                p_next_pos->x = 3 * cube_size + y_in_cube;
                p_next_pos->y = 2 * cube_size;
                *p_next_facing = Facing::Down;
            
            } else if (i_face == 2 && start_facing == Facing::Up && y_in_cube == 0) {
                p_next_pos->x = cube_size - 1 - x_in_cube;
                p_next_pos->y = 0;
                *p_next_facing = Facing::Down;
            
            } else if (i_face == 2 && start_facing == Facing::Down && y_in_cube == cube_size - 1) {
                p_next_pos->x = cube_size - 1 - x_in_cube;
                p_next_pos->y = 3 * cube_size - 1;
                *p_next_facing = Facing::Up;
            
            } else if (i_face == 2 && start_facing == Facing::Left && x_in_cube == 0) {
                p_next_pos->x = 4 * cube_size - 1;
                p_next_pos->y = 2 * cube_size + y_in_cube; //CHECK
                *p_next_facing = Facing::Left;
            
            } else if (i_face == 3 && start_facing == Facing::Up && y_in_cube == 0) {
                p_next_pos->x = 2 * cube_size;
                p_next_pos->y = x_in_cube;
                *p_next_facing = Facing::Left;
            
            } else if (i_face == 3 && start_facing == Facing::Down && y_in_cube == cube_size - 1) {
                p_next_pos->x = 2 * cube_size;
                p_next_pos->y = 2 * cube_size + x_in_cube;
                *p_next_facing = Facing::Left;
            
            } else if (i_face == 4 && start_facing == Facing::Right && x_in_cube == cube_size - 1) {
                p_next_pos->x = 3 * cube_size + (cube_size-1) - y_in_cube;
                p_next_pos->y = 2 * cube_size;
                *p_next_facing = Facing::Down;
            
            } else if (i_face == 5 && start_facing == Facing::Down && y_in_cube == cube_size - 1) {
                p_next_pos->x = cube_size - 1 - x_in_cube;
                p_next_pos->y = 2*cube_size - 1;
                *p_next_facing = Facing::Up;
            
            } else if (i_face == 5 && start_facing == Facing::Left && x_in_cube == 0) {
                p_next_pos->x = cube_size + (cube_size -1) - y_in_cube;
                p_next_pos->y = 2 * cube_size - 1;
                *p_next_facing = Facing::Up;
            
            } else if (i_face == 6 && start_facing == Facing::Right && x_in_cube == cube_size - 1) {
                p_next_pos->x = 3 * cube_size - 1;
                p_next_pos->y = cube_size - 1 - y_in_cube;
                *p_next_facing = Facing::Left;
            
            } else if (i_face == 6 && start_facing == Facing::Up && y_in_cube == 0) {
                p_next_pos->x = 3 * cube_size - 1;
                p_next_pos->y = cube_size - 1 - x_in_cube;
                *p_next_facing = Facing::Left;
            
            } else if (i_face == 6 && start_facing == Facing::Down && y_in_cube == cube_size - 1) {
                p_next_pos->x = 0;
                p_next_pos->y = cube_size + y_in_cube; //CHECK
                *p_next_facing = Facing::Left;
            
            } else {
                assert(false);
            }

        }


        void fold_input(GridPos start_pos, Facing start_facing, GridPos* p_next_pos, Facing* p_next_facing) {
            int i_face;
            
            const int cube_size = 50;
            
            int x_in_cube = start_pos.x % cube_size;
            int y_in_cube = start_pos.y % cube_size;

            if (start_pos.y < cube_size) {
                if (start_pos.x < 2*cube_size)
                    i_face = 2;
                else
                    i_face = 1;
            
            } else if (start_pos.y < 2*cube_size) {
                i_face = 3;

            } else if (start_pos.y < 3*cube_size) {
                if (start_pos.x < cube_size)
                    i_face = 5;
                else
                    i_face = 4;
            
            } else {
                i_face = 6;
            }
            
            if (trace) cout << "start_pos: " << start_pos << endl;
            if (trace) cout << "face: " << i_face << " start_facing: " << start_facing <<
                 " x_in_cube: " << x_in_cube << " y_in_cube: " << y_in_cube << endl;

            if (i_face == 1 && start_facing == Facing::Up && y_in_cube == 0) {
                //to face 6
                p_next_pos->x = x_in_cube;
                p_next_pos->y = 4 * cube_size - 1;
                *p_next_facing = Facing::Up;

            } else if (i_face == 1 && start_facing == Facing::Right && x_in_cube == cube_size - 1) {
                //to face 4
                p_next_pos->x = 2 * cube_size - 1;
                p_next_pos->y = 2 * cube_size + cube_size-1 - y_in_cube;
                *p_next_facing = Facing::Left;
            
            } else if (i_face == 1 && start_facing == Facing::Down && y_in_cube == cube_size - 1) {
                //to face 3
                p_next_pos->x = 2 * cube_size - 1;
                p_next_pos->y = cube_size + x_in_cube;
                *p_next_facing = Facing::Left;
            
            } else if (i_face == 2 && start_facing == Facing::Up && y_in_cube == 0) {
                //to face 6
                p_next_pos->x = 0;
                p_next_pos->y = 3 * cube_size + x_in_cube;
                *p_next_facing = Facing::Right;
            
            } else if (i_face == 2 && start_facing == Facing::Left && x_in_cube == 0) {
                //to face 5
                p_next_pos->x = 0;
                p_next_pos->y = 2 * cube_size + cube_size-1 - y_in_cube;
                *p_next_facing = Facing::Right;
            
            } else if (i_face == 3 && start_facing == Facing::Left && x_in_cube == 0) {
                //to face 5
                p_next_pos->x = y_in_cube;
                p_next_pos->y = 2*cube_size;
                *p_next_facing = Facing::Down;
            
            } else if (i_face == 3 && start_facing == Facing::Right && x_in_cube == cube_size - 1) {
                //to face 1
                p_next_pos->x = 2 * cube_size + y_in_cube;
                p_next_pos->y = cube_size - 1;
                *p_next_facing = Facing::Up;
            
            } else if (i_face == 4 && start_facing == Facing::Right && x_in_cube == cube_size - 1) {
                //to face 1
                p_next_pos->x = 3 * cube_size - 1;
                p_next_pos->y = cube_size-1 - y_in_cube;
                *p_next_facing = Facing::Left;
            
            } else if (i_face == 4 && start_facing == Facing::Down && y_in_cube == cube_size - 1) {
                //to face 6
                p_next_pos->x = cube_size - 1;
                p_next_pos->y = 3 * cube_size + x_in_cube;
                *p_next_facing = Facing::Left;
            
            } else if (i_face == 5 && start_facing == Facing::Up && y_in_cube == 0) {
                //to face 3
                p_next_pos->x = cube_size;
                p_next_pos->y = cube_size + x_in_cube;
                *p_next_facing = Facing::Right;
            
            } else if (i_face == 5 && start_facing == Facing::Left && x_in_cube == 0) {
                //to face 2
                p_next_pos->x = cube_size;
                p_next_pos->y = cube_size-1 - y_in_cube;
                *p_next_facing = Facing::Right;
            
            } else if (i_face == 6 && start_facing == Facing::Right && x_in_cube == cube_size - 1) {
                //to face 4
                p_next_pos->x = cube_size + y_in_cube;
                p_next_pos->y = 3 * cube_size - 1;
                *p_next_facing = Facing::Up;
            
            } else if (i_face == 6 && start_facing == Facing::Left && x_in_cube == 0) {
                //to face 2
                p_next_pos->x = cube_size + y_in_cube;
                p_next_pos->y = 0;
                *p_next_facing = Facing::Down;
            
            } else if (i_face == 6 && start_facing == Facing::Down && y_in_cube == cube_size - 1) {
                //to face 1
                p_next_pos->x = 2 * cube_size + x_in_cube;
                p_next_pos->y = 0;
                *p_next_facing = Facing::Down;
            
            } else {
                assert(false);
            }

        }


        int curr_adding_y = 0;
        vector<vector<TileType>> tiles;
        vector<int> offsets;
        string path;
        Facing curr_facing;
        GridPos curr_pos;
};


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    
    MonkeyMap monkey_map;

    while (true)
    {
        getline(f, line);

        if (line == "") {
            //map is finished
            string path;
            getline(f, path);

            monkey_map.set_path(path);
            break;
        
        } else {
            monkey_map.add_row(line);
        }
    }

    clock_t t_start = clock();

    cout << "performing path" << endl;
    monkey_map.perform_path();

    auto pos = monkey_map.get_pos();
    auto facing = monkey_map.get_facing();
    int total = (pos.y+1) * 1000 + (pos.x+1) * 4 + facing;

    clock_t t_end = clock();

    cout << "result: " << total << endl;
    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
