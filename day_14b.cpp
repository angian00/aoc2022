#include <climits>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>


using namespace std;



struct GridPos {
    int x;
    int y;

    static GridPos parse(string s) {
        //cout << "GridPos.parse(" << s << ")" << "\n";
        size_t sep_pos = s.find(",");

        int x = stoi(s.substr(0, sep_pos));
        int y = stoi(s.substr(sep_pos+1));
        
        return GridPos(x, y);
    }

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


struct GridSegment {
    GridSegment(GridPos start, GridPos end): start(start), end(end) {}

    GridPos start;
    GridPos end;
};


class RockPath {
    public:
        static RockPath parse(string s) {
            RockPath path;
            
            string sep_token = " -> ";
            
            size_t start_index = 0;
            size_t end_index;
            size_t sep_index;
            
            GridPos curr_pos;
            GridPos next_pos;

            do {
                sep_index = s.find(sep_token, start_index);
                end_index = (sep_index == string::npos ? s.length() : sep_index);

                next_pos = GridPos::parse(s.substr(start_index, end_index-start_index));
                if (start_index > 0) {
                    //cout << "Adding segment from " << curr_pos << " to " << next_pos << "\n";
                    path.add_segment(curr_pos, next_pos);
                }

                curr_pos = next_pos;
                start_index = end_index + sep_token.length();
            
            } while (sep_index != string::npos);

            return path;
        }

        void add_segment(GridPos start, GridPos end) {
            segments.push_back(GridSegment(start, end));
        }

        const vector<GridSegment> get_segments() {
            return segments;
        }

        int get_min_x() {
            int res = INT_MAX;
            for (auto segment: segments) {
                if (segment.start.x < res)
                    res = segment.start.x;

                if (segment.end.x < res)
                    res = segment.end.x;
            }

            return res;
        }

        int get_max_x() {
            int res = INT_MIN;
            for (auto segment: segments) {
                if (segment.start.x > res)
                    res = segment.start.x;

                if (segment.end.x > res)
                    res = segment.end.x;
            }

            return res;
        }

        int get_min_y() {
            int res = INT_MAX;
            for (auto segment: segments) {
                if (segment.start.y < res)
                    res = segment.start.y;

                if (segment.end.y < res)
                    res = segment.end.y;
            }

            return res;
        }

        int get_max_y() {
            int res = INT_MIN;
            for (auto segment: segments) {
                if (segment.start.y > res)
                    res = segment.start.y;

                if (segment.end.y > res)
                    res = segment.end.y;
            }

            return res;
        }

    private:
        vector<GridSegment> segments;
};


const GridPos pour_pos(500, 0);


class SandMap {
    public:
        void add_rock_path(RockPath rock_path) {
            rock_paths.push_back(rock_path);
        }

        void update_bounds() {
            //cout << "update_bounds()" << endl;
        
            x_min = INT_MAX;
            x_max = INT_MIN;
            y_min = INT_MAX;
            y_max = INT_MIN;

            for (auto rock_path: rock_paths) {
                int path_left = rock_path.get_min_x();
                if (path_left < x_min)
                    x_min = path_left;

                int path_right = rock_path.get_max_x();
                if (path_right > x_max)
                    x_max = path_right;

                int path_top = rock_path.get_min_y();
                if (path_top < y_min)
                    y_min = path_top;

                int path_bottom = rock_path.get_max_y();
                if (path_bottom > y_max)
                    y_max = path_bottom;
            }

            //cout << "after mins()" << endl;
            //margins for sand accumulation
            x_min -= 100000;
            x_max += 100000;
            y_min = 0;
            y_max += 2;

            int w = x_max - x_min + 1;
            int h = y_max - y_min + 1;
            filled = new bool[w*h];
            //cout << "after assigning filled" << endl;

            for (int y=0; y < h; y++) {
                for (int x=0; x < w; x++) {
                    if (y < h - 1) 
                        filled[y*w + x] = false;
                    else
                        //hard rock on bottom
                        filled[y*w + x] = true;
                }
            }

            //cout << "after setting all false" << endl;

            for (auto rock_path: rock_paths) {
                for (auto segment: rock_path.get_segments()) {
                    if (segment.start.x == segment.end.x) {
                        int x = segment.start.x;
                        if (segment.end.y > segment.start.y) {
                            for (int y=segment.start.y; y <= segment.end.y; y++) {
                                filled[(y-y_min)*w + (x-x_min)] = true;
                            }
                        } else {
                            for (int y=segment.start.y; y >= segment.end.y; y--) {
                                filled[(y-y_min)*w + (x-x_min)] = true;
                            }
                        } 

                    } else {
                        int y = segment.start.y;
                        if (segment.end.x > segment.start.x) {
                            for (int x=segment.start.x; x <= segment.end.x; x++) {
                                filled[(y-y_min)*w + (x-x_min)] = true;
                            }
                        } else {
                            for (int x=segment.start.x; x >= segment.end.x; x--) {
                                filled[(y-y_min)*w + (x-x_min)] = true;
                            }
                        } 

                    }
                }
            }
        }

        void print() {
            cout << "x from " << x_min << " to " << x_max << endl;
            cout << "y from " << y_min << " to " << y_max << endl;

            for (int y=0; y <= y_max-y_min; y++ ) {
                for (int x=0; x <= x_max-x_min; x++ ) {
                    if (filled[y*(x_max-x_min+1) + x])
                        cout << "#";
                    else
                        cout << ".";
                }
                cout << endl;

            }
        }


        int pour_sand() {
            int n_sand = 0;
            while (pour_one_sand()) {
                n_sand++;
                //cout << "n_sand: " << n_sand << endl;
            }
            
            return n_sand;
        }


        bool pour_one_sand() {
            GridPos curr_pos = pour_pos;
            GridPos next_pos = curr_pos;

            while (curr_pos.y <= y_max) {
                //cout << "next_pos: " << next_pos << endl;

                next_pos.x = curr_pos.x;
                next_pos.y = curr_pos.y + 1;
                if (!is_filled(next_pos)) {
                    curr_pos = next_pos;
                    continue;
                }

                next_pos.x = curr_pos.x - 1;
                next_pos.y = curr_pos.y + 1;
                if (!is_filled(next_pos)) {
                    curr_pos = next_pos;
                    continue;
                }

                next_pos.x = curr_pos.x + 1;
                next_pos.y = curr_pos.y + 1;
                if (!is_filled(next_pos)) {
                    curr_pos = next_pos;
                    continue;
                }

                if (curr_pos == pour_pos)
                    return false;

                set_filled(curr_pos);
                return true;
            }

            return false;
        }


    private:
        bool is_filled(GridPos pos) {
            return filled[(pos.y - y_min) * (x_max - x_min + 1) + pos.x - x_min];
        }

        void set_filled(GridPos pos) {
            filled[(pos.y - y_min) * (x_max - x_min + 1) + pos.x - x_min] = true;
        }

        vector<RockPath> rock_paths;
        bool *filled;
        int x_min;
        int x_max;
        int y_min;
        int y_max;
};


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    SandMap sand_map;
    while (getline(f, line))
    {
        sand_map.add_rock_path(RockPath::parse(line));
    }

    sand_map.update_bounds();
    //sand_map.print();

    cout << endl;

    int n_sand = sand_map.pour_sand();
    
    //sand_map.print();
    cout << "n_sand: " << (n_sand+1) << endl;

    return 0;
}
