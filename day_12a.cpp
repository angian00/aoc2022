#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <climits>

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



GridPos find_lowest_f(set<GridPos> open_set, map<GridPos, int> f_scores) {
    //FIXME: optimize by changing f_score data structure
    int min_f_score = INT_MAX;
    GridPos min_pos;

    for (auto pos_it = open_set.begin(); pos_it != open_set.end(); pos_it++) {
        GridPos curr_pos = *pos_it;
        if (f_scores[curr_pos] < min_f_score) {
            min_f_score = f_scores[curr_pos];
            min_pos = curr_pos;
        }
    }

    return min_pos;
}


vector<GridPos> reconstruct_path(map<GridPos, GridPos> came_from, GridPos end_pos) {
    vector<GridPos> res;

    GridPos curr_pos = end_pos;
    while (came_from.contains(curr_pos)) {
        //cout << "-- reconstruct_path: found node " << curr_pos << endl;
        res.push_back(curr_pos);

        curr_pos = came_from[curr_pos];
    }

    return res;
}


class PathResolver {
    
    public:
        PathResolver(int map_width, int map_height, map<GridPos, int> elevation_map, GridPos start_pos, GridPos end_pos): 
            map_width(map_width), map_height(map_height), elevation_map(elevation_map), start_pos(start_pos), end_pos(end_pos) {}


        int compute_path() {
            int path_len = 0;

            set<GridPos> open_set;
            open_set.insert(start_pos);

            map<GridPos, GridPos> came_from;
        
            map<GridPos, int> g_score;
            map<GridPos, int> f_score;
            for (auto pos_it = elevation_map.begin(); pos_it != elevation_map.end(); pos_it++) {
                g_score[pos_it->first] = INT_MAX;
                f_score[pos_it->first] = INT_MAX;
            }

            g_score[start_pos] = 0;
            f_score[start_pos] = heuristic(start_pos);

            while (!open_set.empty()) {
                //cout << endl;
                //cout << "-- open_set iteration" << endl;

                GridPos curr_pos = find_lowest_f(open_set, f_score);
                //cout << "-- Processing node " << curr_pos << endl;

                if (curr_pos == end_pos) {
                    //cout << "-- Found final node " << endl;
                    //return -1; //DEBUG

                    vector<GridPos> full_path = reconstruct_path(came_from, curr_pos);
                    //print_path(full_path);
                    return full_path.size();
                }

                //cout << "-- Removing node " << curr_pos << " from open_set" << endl;
                open_set.erase(curr_pos);

                //cout << "-- Finding neighbors  for " << curr_pos << endl;
                set<GridPos> neighbors = find_neighbors(curr_pos);
                for (auto neighbor_it=neighbors.begin(); neighbor_it != neighbors.end(); neighbor_it++) {
                    GridPos neighbor = *neighbor_it;
                    //cout << "-- Processing neighbor " << neighbor << endl;
                    int tentative_g_score = g_score[curr_pos] + distance(curr_pos, neighbor);
                    if (tentative_g_score < g_score[neighbor]) {
                        //cout << "-- Better neighbor " << neighbor << " for node " << curr_pos << 
                        //        " g_score from " << g_score[neighbor] << " to " << tentative_g_score << endl;

                        came_from[neighbor] = curr_pos;
                        g_score[neighbor] = tentative_g_score;
                        f_score[neighbor] = heuristic(neighbor);
                        if (!open_set.contains(neighbor)) {
                            //cout << "-- Adding neighbor " << neighbor << " to open_set" << endl;
                            open_set.insert(neighbor);
                        }
                    }
                }
            }

            return -1;
        }
    
        void print_elevation() {
            for (int y=0; y < map_height; y++) {
                for (int x=0; x < map_width; x++) {
                    GridPos pos(x, y);
                    //cout << pos << ": " << elevation_map[pos] << " ";
                    cout << elevation_map[pos] << " ";
                }

                cout << endl;
            }

            cout << endl;
        }


    private:
        int heuristic(GridPos pos) {
            return  - (abs(end_pos.x - pos.x) + abs(end_pos.y - pos.y));
            //return elevation_map[end_pos] - elevation_map[pos];
        }

        set<GridPos> find_neighbors(GridPos curr_pos) {
            set<GridPos> res;
            GridPos neighbor;

            neighbor.x = curr_pos.x - 1;
            neighbor.y = curr_pos.y;
            //if (elevation_map.contains(neighbor) && distance(curr_pos, neighbor) == 1)
            if (elevation_map.contains(neighbor))
                res.insert(neighbor);

            neighbor.x = curr_pos.x + 1;
            neighbor.y = curr_pos.y;
            //if (elevation_map.contains(neighbor) && distance(curr_pos, neighbor) == 1)
            if (elevation_map.contains(neighbor))
                res.insert(neighbor);

            neighbor.x = curr_pos.x;
            neighbor.y = curr_pos.y - 1;
            //if (elevation_map.contains(neighbor) && distance(curr_pos, neighbor) == 1)
            if (elevation_map.contains(neighbor))
                res.insert(neighbor);

            neighbor.x = curr_pos.x;
            neighbor.y = curr_pos.y + 1;
            //if (elevation_map.contains(neighbor) && distance(curr_pos, neighbor) == 1)
            if (elevation_map.contains(neighbor))
                res.insert(neighbor);

            return res;
        }

        int distance(GridPos from, GridPos to) {
            if (elevation_map[from] < elevation_map[to] - 1)
                return 999999;
            else
                return 1;
        }


        static void print_path(vector<GridPos> map_path) {
            cout << "Map path: " << endl;

            for (auto path_it=map_path.begin(); path_it != map_path.end(); path_it++) {
                cout << "\t" << (*path_it) << endl;
            }
        }

        map<GridPos, int> elevation_map;
        int map_width;
        int map_height;
        GridPos start_pos;
        GridPos end_pos;
};


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    map<GridPos, int> elevation_map;
    GridPos start_pos;
    GridPos end_pos;
    GridPos curr_pos;
    int w;
    int h;

    int y = 0;
    while (getline(f, line))
    {
        w = line.length();
        for (int x=0; x < w; x++) {
            char c = line[x];
            //cout << "Read char: " << c << endl;

            if (c == 'S') {
                start_pos.x = x;
                start_pos.y = y;
                c = 'a';
            } else if (c == 'E') {
                end_pos.x = x;
                end_pos.y = y;
                c = 'z';
            }

            //cout << "After S/E: " << c << endl;

            GridPos curr_pos;
            curr_pos.x = x;
            curr_pos.y = y;
            elevation_map[curr_pos] = (c - 'a');
        }

        y++;
    }
    h = y;


    PathResolver pr(w, h, elevation_map, start_pos, end_pos);
    //pr.print_elevation();
    int path_len = pr.compute_path();
    cout << path_len << "\n";

    return 0;
}
