#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <deque>
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


vector<GridPos> find_lowest_positions(map<GridPos, int> elevation_map) {
    vector<GridPos> res;

    for (auto pos_it = elevation_map.begin(); pos_it != elevation_map.end(); pos_it ++) {
        if (elevation_map[pos_it->first] == 0)
            res.push_back(pos_it->first);
    }

    return res;
}


class PathResolver {
    public:
        PathResolver(int map_width, int map_height, map<GridPos, int> elevation_map): 
            map_width(map_width), map_height(map_height), elevation_map(elevation_map) {}


        void compute_paths(GridPos start_pos) {
            this->start_pos = start_pos;

            deque<GridPos> open_set;
            map<GridPos, bool> explored;

            for (auto pos_it=elevation_map.begin(); pos_it != elevation_map.end(); pos_it++) {
                GridPos curr_pos = pos_it->first;
                distances[curr_pos] = 999999;
                explored[curr_pos] = false;
            }
            
            open_set.push_back(start_pos);

            distances[start_pos] = 0;
            while (!open_set.empty()) {
                //cout << "-- open_set iteration" << endl;

                GridPos curr_pos = open_set.front();
                open_set.pop_front();

                //cout << "-- curr_pos=" << curr_pos << endl;

                set<GridPos> neighbors = find_neighbors(curr_pos);
                for (auto neighbor_it=neighbors.begin(); neighbor_it != neighbors.end(); neighbor_it++) {
                    GridPos neighbor = *neighbor_it;
                    //cout << "-- neighbor=" << neighbor << endl;

                    if (!explored[neighbor]) {
                        explored[neighbor] = true;
                        int alt = distances[curr_pos] + 1;
                        //cout << "-- alt=" << alt << endl;
                        if (alt < distances[neighbor]) {
                            distances[neighbor] = alt;
                            came_from[neighbor] = curr_pos;
                        }

                        open_set.push_back(neighbor);
                    }

                }
                
            }

        }
    
        int get_path_len(GridPos end_pos) {
            return distances[end_pos];
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
        set<GridPos> find_neighbors(GridPos curr_pos) {
            set<GridPos> res;
            GridPos neighbor;

            //cout << "find_neighbors; curr_pos=" << curr_pos << endl;

            neighbor.x = curr_pos.x - 1;
            neighbor.y = curr_pos.y;
            //cout << "find_neighbors; neighbor=" << neighbor << ", distance=" << distance(curr_pos, neighbor) << endl;
            if (elevation_map.contains(neighbor) && distance(curr_pos, neighbor) == 1)
                res.insert(neighbor);

            neighbor.x = curr_pos.x + 1;
            neighbor.y = curr_pos.y;
            if (elevation_map.contains(neighbor) && distance(curr_pos, neighbor) == 1)
                res.insert(neighbor);

            neighbor.x = curr_pos.x;
            neighbor.y = curr_pos.y - 1;
            if (elevation_map.contains(neighbor) && distance(curr_pos, neighbor) == 1)
                res.insert(neighbor);

            neighbor.x = curr_pos.x;
            neighbor.y = curr_pos.y + 1;
            if (elevation_map.contains(neighbor) && distance(curr_pos, neighbor) == 1)
                res.insert(neighbor);

            return res;
        }

        int distance(GridPos from, GridPos to) {
            //cout << "find_neighbors; from=" << from << ", to=" << to << 
            //    ", elevation from: " << elevation_map[from] << ", elevation to: " << elevation_map[to] << endl;

            if (elevation_map[from] > elevation_map[to] + 1) //reversed wrt formulation of problem
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

        map<GridPos, int> distances;
        map<GridPos, GridPos> came_from;
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


    PathResolver pr(w, h, elevation_map);
    pr.compute_paths(end_pos); //start_end and end_pos are reversed due to formulation of problem vs Dijkstra algo

    vector<GridPos> start_positions = find_lowest_positions(elevation_map);

    int min_path_len;
    for (int i=0; i < start_positions.size(); i++) {
        int path_len = pr.get_path_len(start_positions[i]);
        //cout << "path_len for position # " << i << " : " << path_len << endl;
        if (path_len < min_path_len)
            min_path_len = path_len;
    }


    //pr.print_elevation();
    cout << min_path_len << "\n";

    return 0;
}
