#include <iostream>
#include <fstream>
#include <string>
#include <vector>


using namespace std;


const int n_rocks = 2022;
const int field_width = 7;


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

    int w;
    int h;
    bool **solid_tiles;
};

struct Rock {
    GridPos curr_pos;
    bool is_alive;
    RockPattern pattern;
};


class PlayField {
    public:
        PlayField() {
            width = field_width;
            height_solid = 0;
            i_pattern = 0;
            i_rock = 0;
        }

        void play(int n_rocks, string moves) {
            int i_move = 0;

            create_active_rock();
            //print();

            while (i_rock < n_rocks)
            {
                //cout << "i_move=" << i_move << endl;
                //print();

                char next_move = moves[i_move % moves.length()];
                i_move++;

                //cout << "Before move_rock_left_right" << endl;
                move_rock_left_right(next_move == '>');
                //print();

                //cout << "Before move_rock_down" << endl;
                move_rock_down();
                //if (!move_rock_down())
                //    print();
            }
        }

        int get_height_solid() {
            return height_solid;
        }

        void print() {
            for (int y=tiles.size()-1; y >= 0; y--) {
                auto tile_row = tiles[y];
                //cout << "Inside external loop; y=" << y << endl;
                for (int x=0; x < tile_row.size(); x++) {
                    char c;
                    //cout << "Inside internal loop" << endl;
                    //cout << "pattern coords: x=" << x-active_rock->curr_pos.x <<
                    //    ", y=" << y-active_rock->curr_pos.y << endl;

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
            
            active_rock = new Rock();
            active_rock->is_alive = true;
            active_rock->curr_pos = GridPos(2, height_solid + 3);
            active_rock->pattern = RockPattern::create_pattern(rock_patterns[i_pattern % (sizeof(rock_patterns)/sizeof(RockPatternName))]);
            i_pattern ++;
            i_rock ++;

            expand_tiles();
        }

        void expand_tiles() {
            int n_rows_before = tiles.size();
            for (int i=0; i < active_rock->curr_pos.y + active_rock->pattern.h - n_rows_before; i++) {
                vector<bool> tile_row;
                for (int x=0; x < field_width; x++)
                    tile_row.push_back(false);
                tiles.push_back(tile_row);
                //cout << "tile_row size: " << tile_row.size() << endl;
            }
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
                    if (tiles[new_pos.y + y][new_pos.x + x] && active_rock->pattern.solid_tiles[y][x]) {
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
                        tiles[active_rock->curr_pos.y + y][active_rock->curr_pos.x + x] = true;
                }
            }

            int new_height_solid = active_rock->curr_pos.y + active_rock->pattern.h;
            if (new_height_solid > height_solid)
                height_solid = new_height_solid;

            free(active_rock);
        }

        int width;
        int height_solid;
        int i_rock;

        vector<vector<bool>> tiles;
        Rock* active_rock;
        int i_pattern;
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
    //field.play(11, moves);
    field.play(2023, moves);
    //field.print();

    cout << "reached height: " << field.get_height_solid() << endl;

    return 0;
}
