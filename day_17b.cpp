#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <assert.h>


using namespace std;


const int field_width = 7;
const int window_size = 1000;
const int window_offset = 50;
const int n_check_history = 200;
//const int n_check_history = 10000;


typedef long long GridCoord;

struct GridPos {
    GridCoord x;
    GridCoord y;

    GridPos(GridCoord x, GridCoord y): x(x), y(y) {}
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


enum RockPatternName {
    MINUS,
    PLUS,
    LETTER_L,
    LETTER_I,
    SQUARE
};

RockPatternName rock_patterns[] = {
    MINUS,
    PLUS,
    LETTER_L,
    LETTER_I,
    SQUARE
};


int lcm (int a, int b)
{
    int res;

    if (a > b)
        res = a;
    else
        res = b;

    while (true) {
        if ((res % a == 0) && (res % b == 0)) {
            return res;
        }

        res ++;
    }
}

struct RockPattern {

    static RockPattern create_pattern(RockPatternName name) {
        RockPattern new_pattern;

        switch (name) {
            case MINUS:
                new_pattern.w = 4;
                new_pattern.h = 1;
                new_pattern.init_tiles();

                new_pattern.solid_tiles[0][0] = true;
                new_pattern.solid_tiles[0][1] = true;
                new_pattern.solid_tiles[0][2] = true;
                new_pattern.solid_tiles[0][3] = true;
                break;

            case PLUS:
                new_pattern.w = 3;
                new_pattern.h = 3;
                new_pattern.init_tiles();

                new_pattern.solid_tiles[0][1] = true;
                new_pattern.solid_tiles[1][0] = true;
                new_pattern.solid_tiles[1][1] = true;
                new_pattern.solid_tiles[1][2] = true;
                new_pattern.solid_tiles[2][1] = true;
                break;

            case LETTER_L:
                new_pattern.w = 3;
                new_pattern.h = 3;
                new_pattern.init_tiles();

                new_pattern.solid_tiles[0][0] = true;
                new_pattern.solid_tiles[0][1] = true;
                new_pattern.solid_tiles[0][2] = true;
                new_pattern.solid_tiles[1][2] = true;
                new_pattern.solid_tiles[2][2] = true;
                break;

            case LETTER_I:
                new_pattern.w = 1;
                new_pattern.h = 4;
                new_pattern.init_tiles();

                new_pattern.solid_tiles[0][0] = true;
                new_pattern.solid_tiles[1][0] = true;
                new_pattern.solid_tiles[2][0] = true;
                new_pattern.solid_tiles[3][0] = true;
                break;

            case SQUARE:
                new_pattern.w = 2;
                new_pattern.h = 2;
                new_pattern.init_tiles();

                new_pattern.solid_tiles[0][0] = true;
                new_pattern.solid_tiles[0][1] = true;
                new_pattern.solid_tiles[1][0] = true;
                new_pattern.solid_tiles[1][1] = true;
                break;
        }

        new_pattern.name = name;
        return new_pattern;
    }

    void init_tiles() {
        solid_tiles = new bool*[h];

        for (int y=0; y < h; y++) {
            solid_tiles[y] = new bool[w];
            for (int x=0; x < w; x++) {
                solid_tiles[y][x] = false;
            }
        }
    }

    RockPatternName name;
    int w;
    int h;
    bool **solid_tiles;
};

struct Rock {
    GridPos curr_pos;
    bool is_alive;
    RockPattern pattern;
};


struct HistoryItem {
    GridCoord height_solid;
    GridCoord i_rock;
    RockPatternName pattern;
    int i_move;
};


class PlayField {
    public:
        PlayField() {
            width = field_width;
            height_solid = 0;
            i_pattern = 0;
            i_rock = 0;
            i_move = 0;
            y_offset = 0;
        }

        void play(GridCoord n_rocks, string moves, bool loop_check_enabled) {
            GridCoord computed_height_solid = 0;
            int tail_n_rocks = -1;
            int tail_height_solid_start = 0;
            int head_n_rocks = -1;


            create_active_rock();
            //print();

            while (i_rock < n_rocks)
            {
                //cout << "i_move=" << i_move << endl;
                //print();

                char next_move = moves[i_move];
                i_move++;
                if (i_move == moves.length())
                    i_move -= moves.length();

                move_rock_left_right(next_move == '>');

                bool could_move_rock = move_rock_down();

                if (loop_check_enabled && !could_move_rock && tail_n_rocks != -1) {
                    tail_n_rocks --;
                    if (tail_n_rocks < 0) {
                        cout << "Stopping; i_rock=" << i_rock << ", height_solid=" << height_solid << endl;
                        break;
                    }
                }

                if (loop_check_enabled && !could_move_rock && (tail_n_rocks == -1) && (i_rock % n_check_history == 0)) {
                    // check for loops
                    int start_loop_index = check_history();

                    if (start_loop_index != -1) {
                        auto start_loop_item = history[start_loop_index];
                        if (head_n_rocks != -1 && head_n_rocks != start_loop_item.i_rock)
                            continue;

                        cout << "Found history repeating between i_rock=" << start_loop_item.i_rock << " and i_rock=" << i_rock 
                            <<", heights " << start_loop_item.height_solid << " and " << height_solid << endl;
                        
                        head_n_rocks = start_loop_item.i_rock;
                        auto head_height_solid = start_loop_item.height_solid;
                        auto loop_n_rocks = i_rock - head_n_rocks;

                        auto loop_height_solid = height_solid - head_height_solid;
                        auto n_loops = (n_rocks - head_n_rocks) / loop_n_rocks;
                        computed_height_solid = n_loops * loop_height_solid + head_height_solid;
                        //assert(tail_n_rocks == ((n_rocks - head_n_rocks) % n_loops)); //OK
                        tail_n_rocks = n_rocks - head_n_rocks - n_loops * loop_n_rocks;
                        tail_height_solid_start = height_solid;

                        cout << "i_move=" << i_move << endl;
                        cout << "moves.length=" << moves.length() << endl;
                        cout << endl;
                        cout << "n_rocks=" << n_rocks << endl;
                        cout << "window_size=" << window_size << endl;
                        cout << "window_offset=" << window_offset << endl;
                        cout << endl;
                        cout << "first repetition i_rock=" << i_rock << endl;
                        cout << "head_n_rocks=" << head_n_rocks << endl;
                        cout << "head_height_solid=" << head_height_solid << endl;
                        cout << "loop_n_rocks=" << loop_n_rocks << endl;
                        cout << "loop_height_solid=" << loop_height_solid << endl;
                        cout << "n_loops=" << n_loops << endl;
                        cout << "tail_n_rocks=" << tail_n_rocks << endl;
                        cout << "tail_height_solid_start=" << tail_height_solid_start << endl;

                        //print(head_height_solid-window_offset - 40);

                        if (tail_n_rocks == 0) {
                            cout << "Stopping immediately; i_rock=" << i_rock << ", height_solid=" << height_solid << endl;
                            break;
                        }
                    }
                }

                if (i_rock % 5000 == 0) {
                    //update_y_offset();
                }
            }
            
            cout << "Finished play; i_move=" << i_move << endl;

            if (tail_height_solid_start != 0) {
                computed_height_solid += height_solid - tail_height_solid_start;
                height_solid = computed_height_solid;
            }
        }


        GridCoord get_height_solid() {
            return height_solid;
        }

        void print(GridCoord start_y=0) {
            cout << "(y_offset: " << y_offset << ")" << endl;

            if (start_y < 0)
                start_y += tiles.size();

            for (int y=tiles.size()-1; y >= start_y; y--) {
                auto tile_row = tiles[y];
                //cout << "Inside external loop; y=" << y << endl;
                for (int x=0; x < tile_row.size(); x++) {
                    char c;

                    if (rock_occupies_pos(GridPos(x, y)))
                        c = '@';
                    else if (tiles[y][x])
                        c = '#';
                    else
                        c = '.';

                    cout << c;
                }

                cout << endl;
            }
            
            cout << endl;
        }


    private:
        void create_active_rock() {
            //cout << "create_active_rock" << endl;
            //cout << "height_solid=" << height_solid << endl;
            
            RockPatternName new_pattern = rock_patterns[i_pattern % (sizeof(rock_patterns)/sizeof(RockPatternName))];
            history.push_back(HistoryItem(height_solid, i_rock, new_pattern, i_move));

            active_rock = new Rock();
            active_rock->is_alive = true;
            active_rock->curr_pos = GridPos(2, height_solid + 3);
            active_rock->pattern = RockPattern::create_pattern(new_pattern);
            i_pattern ++;
            i_rock ++;

            expand_tiles();
        }

        void expand_tiles() {
            int n_rows_before = tiles.size();
            for (int i=0; i < active_rock->curr_pos.y - y_offset + active_rock->pattern.h - n_rows_before; i++) {
                vector<bool> tile_row;
                for (int x=0; x < width; x++)
                    tile_row.push_back(false);
                tiles.push_back(tile_row);
                //cout << "tile_row size: " << tile_row.size() << endl;
            }
        }

        bool update_y_offset() {
            //look for configurations like:
            // ###.###
            // ...#...

            int delta_offset = 0;

            for (int y=tiles.size()-1; y > 0; y--) {
                auto tile_row = tiles[y];

                int n_free = 0;
                int x_free = -1;
                for (int x=0; x < tile_row.size(); x++) {
                    if (!tile_row[x]) {
                        x_free = x;
                        n_free ++;
                        if (n_free > 1)
                            break;
                    }
                }

                if (n_free == 1 && !tiles[y-1][x_free]) {
                    delta_offset = y;
                    break;
                }
            }

            for (int i=0; i < delta_offset; i++) {
                tiles.pop_front();
                
            }
            
            y_offset += delta_offset;
            //cout << "update_y_offset; delta_offset=" << delta_offset << endl;

            return (delta_offset > 0);
        }

        int check_history() {

            //cout << "check_history" << endl;

            for (int i_item = history.size()-2; i_item >= 0; i_item--) {
                auto history_item = history[i_item];
                if (history_item.height_solid == height_solid)
                    continue;

                if (history_item.pattern != active_rock->pattern.name)
                    continue;
                
                if (history_item.i_move != i_move)
                    continue;
                
                if (history_item.height_solid < window_size + window_offset)
                    continue;

                //cout << "check_history checking" << endl;

                bool repetition_found = true;
                for (int delta_y=window_offset; delta_y < window_offset + window_size; delta_y++) {
                    for (int x=0; x < width; x++) {
                        // if (tiles[height_solid-delta_y][x]) {
                        //     cout << "check_history delta_y=" << delta_y << ", x=" << x << endl;
                        //     cout << "curr:" << tiles[height_solid-delta_y][x] << ", history=" << tiles[history_item.height_solid-delta_y][x] << endl;
                        // }

                        if (tiles[height_solid-delta_y][x] != tiles[history_item.height_solid-delta_y][x]) {
                            repetition_found = false;
                            break;
                        }
                    }

                    if (!repetition_found)
                        break;
                }

                if (repetition_found)
                    return i_item;
            }

            return -1;
        }

        bool move_rock_down() {
            //cout << "move_rock_down" << endl;

            GridPos dest_pos = GridPos(active_rock->curr_pos.x, active_rock->curr_pos.y-1);
            bool can_move = rock_can_move(dest_pos);

            if (can_move) {
                active_rock->curr_pos.y --;
                return true;
            } else {
                consolidate_rock();
                create_active_rock();
                return false;
            }
        }

        bool move_rock_left_right(bool is_right) {
            // if (is_right)
            //     cout << "move_rock_right" << endl;
            // else
            //     cout << "move_rock_left" << endl;

            int hor_step = is_right ? 1 : -1;
            GridPos dest_pos = GridPos(active_rock->curr_pos.x + hor_step, active_rock->curr_pos.y);
            bool can_move = rock_can_move(dest_pos);

            if (!can_move)
                return false;
            
            active_rock->curr_pos = dest_pos;
            return true;
        }

        bool rock_can_move(GridPos new_pos) {
            if (new_pos.x < 0)
                return false;
            if (new_pos.x + active_rock->pattern.w > width)
                return false;
            if (new_pos.y < 0)
                return false;

            for (int y=0; y < active_rock->pattern.h; y++) {
                for (int x=0; x < active_rock->pattern.w; x++) {
                    if (tiles[new_pos.y + y - y_offset][new_pos.x + x] && active_rock->pattern.solid_tiles[y][x]) {
                        return false;
                    }
                }
            }

            return true;
        }

        bool rock_occupies_pos(GridPos pos) {
            if (pos.x < active_rock->curr_pos.x)
                return false;                
            if (pos.x >= active_rock->curr_pos.x + active_rock->pattern.w)
                return false;
                
            if (pos.y < active_rock->curr_pos.y)
                return false;                
            if (pos.y >= active_rock->curr_pos.y + active_rock->pattern.h)
                return false;
                
            return (active_rock->pattern.solid_tiles[pos.y-active_rock->curr_pos.y][pos.x-active_rock->curr_pos.x]);
        }

        void consolidate_rock() {
            for (int y=0; y < active_rock->pattern.h; y++) {
                for (int x=0; x < active_rock->pattern.w; x++) {
                    if (active_rock->pattern.solid_tiles[y][x])
                        tiles[active_rock->curr_pos.y - y_offset + y][active_rock->curr_pos.x + x] = true;
                }
            }

            int new_height_solid = active_rock->curr_pos.y + active_rock->pattern.h;
            if (new_height_solid > height_solid)
                height_solid = new_height_solid;

            free(active_rock);
        }

        int width;
        GridCoord height_solid;
        GridCoord i_rock;
        int i_move;

        deque<vector<bool>> tiles;
        Rock* active_rock;
        int i_pattern;

        vector<HistoryItem> history;
        GridCoord y_offset;
};




int main(int argc, char *argv[])
{
    string filename = argv[1];
    ifstream f(filename);

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    string moves;
    getline(f, moves);

    PlayField field;
    //int n_rocks = 2023;
    //int n_rocks = 1000; //expected: 
    int n_rocks = 100000; //expected: 151434
    //int n_rocks = 1000000; //expected: 1514288
    //int n_rocks = 10000000;
    //int n_rocks = 1000000000000;
    //field.play(11, moves);
    //field.play(n_rocks, moves, true);
    field.play(n_rocks, moves, false);
    //field.print();
    //field.print(-30);

    cout << "reached height: " << field.get_height_solid() << endl;

    return 0;
}
