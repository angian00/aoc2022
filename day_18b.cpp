#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <set>
#include <climits>


using namespace std;

struct GridPos3D {
    int x;
    int y;
    int z;

    GridPos3D(int x, int y, int z): x(x), y(y), z(z) {}
    GridPos3D(): x(-1), y(-1), z(-1) {}

    static GridPos3D parse(string s) {
        static regex parse_regex("(\\d+),(\\d+),(\\d+)");
        smatch matches;
        regex_search(s, matches, parse_regex);

        GridPos3D res;
        res.x = stoi(matches.str(1));
        res.y = stoi(matches.str(2));
        res.z = stoi(matches.str(3));
    
        return res;
    }

    friend bool operator==(const GridPos3D& lhs, const GridPos3D& rhs) { 
        return ( (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z) );
    }

    friend bool operator<(const GridPos3D& lhs, const GridPos3D& rhs)
    {
      return (lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y) || (lhs.x == rhs.x && lhs.y == rhs.y && lhs.z < rhs.z) );
    }

    friend std::ostream & operator<<( std::ostream &os, const GridPos3D &value )
    {
        os << "(" << value.x << ", " << value.y << ", " << value.z << ")";
        
        return os;
    }

};


class Droplet {
    public:
        Droplet() {
            min_x = INT_MAX;
            min_y = INT_MAX;
            min_z = INT_MAX;
            max_x = INT_MIN;
            max_y = INT_MIN;
            max_z = INT_MIN;
        }

        void add_voxel(GridPos3D voxel) {
            solid_voxels.insert(voxel);
            
            if (voxel.x < min_x)
                min_x = voxel.x;
            if (voxel.y < min_y)
                min_y = voxel.y;
            if (voxel.z < min_z)
                min_z = voxel.z;

            if (voxel.x > max_x)
                max_x = voxel.x;
            if (voxel.y > max_y)
                max_y = voxel.y;
            if (voxel.z > max_z)
                max_z = voxel.z;
        }

        int compute_external_surface() {
            compute_external_voxels();

            int ext_surface = 0;

            for (auto voxel: solid_voxels) {
                for (auto neighbor: enumerate_neighbors(voxel)) {
                    if (external_voxels.contains(neighbor))
                        ext_surface ++;
                }
            }

            return ext_surface;
        }

        void print() {
            cout << "# solid_voxels: " << solid_voxels.size() << endl;
            for (int x = min_x-1; x <= max_x+1; x++) {
                for (int y = min_y-1; y <= max_y+1; y++) {
                    for (int z = min_z-1; z <= max_z+1; z++) {
                        char c;
                        GridPos3D voxel(x, y, z);
                        if (solid_voxels.contains(voxel))
                            c = '#';
                        else if (external_voxels.contains(voxel))
                            c = '.';
                        else
                            c = 'I';

                        cout << c;
                    }

                    cout << endl;
                }
                
                cout << endl;
            }

            cout << endl;
        }

    private:
        set<GridPos3D> enumerate_neighbors(GridPos3D pos) {
            set<GridPos3D> res;

            if (pos.x >= min_x)
                res.insert(GridPos3D(pos.x-1, pos.y, pos.z));
            if (pos.x <= max_x)
                res.insert(GridPos3D(pos.x+1, pos.y, pos.z));

            if (pos.y >= min_y)
                res.insert(GridPos3D(pos.x, pos.y-1, pos.z));
            if (pos.y <= max_y)
                res.insert(GridPos3D(pos.x, pos.y+1, pos.z));

            if (pos.z >= min_z)
                res.insert(GridPos3D(pos.x, pos.y, pos.z-1));
            if (pos.z <= max_z)
                res.insert(GridPos3D(pos.x, pos.y, pos.z+1));

            return res;
        }


        void compute_external_voxels() {
            cout << "compute_external_voxels" << endl;
            cout << "x_min=" << min_x << ", x_max=" << max_x << endl;
            cout << "y_min=" << min_y << ", y_max=" << max_y << endl;
            cout << "z_min=" << min_z << ", z_max=" << max_z << endl;

            deque<GridPos3D> processing_queue;

            for (int x = min_x-1; x <= max_x+1; x++) {
                for (int y = min_y-1; y <= max_y+1; y++) {
                    for (int z = min_z-1; z <= max_z+1; z++) {

                        if ( (x == min_x-1) || (x == max_x+1) || 
                            (y == min_y-1) || (y == max_y+1) || 
                            (z == min_z-1) || (z == max_z+1) ) {

                            GridPos3D voxel(x, y, z);
                            external_voxels.insert(voxel);
                            processing_queue.push_back(voxel);
                            //cout << "adding initial external voxel" << voxel << endl;
                        }

                    }
                }
            }

            while (!processing_queue.empty()) {
                auto curr_voxel = processing_queue.front();
                processing_queue.pop_front();

                //cout << "Processing voxel " << curr_voxel << endl;
                for (auto neighbor: enumerate_neighbors(curr_voxel)) {
                    if (!solid_voxels.contains(neighbor) && !external_voxels.contains(neighbor)) {
                        //cout << "Adding new external voxel" << neighbor <<  " because of " << curr_voxel << endl;
                        external_voxels.insert(neighbor);
                        processing_queue.push_back(neighbor);
                    }
                }
            }
        }


        set<GridPos3D> solid_voxels;
        set<GridPos3D> external_voxels;
        vector<vector<vector<bool>>> solid_array;
        vector<vector<vector<bool>>> exterior_array;
        int min_x;
        int min_y;
        int min_z;
        int max_x;
        int max_y;
        int max_z;
};

int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    Droplet droplet;

    while (getline(f, line))
    {
        auto voxel = GridPos3D::parse(line);
        droplet.add_voxel(voxel);
    }

    int res = droplet.compute_external_surface();
    //droplet.print();
    
    cout << "external surface=" << res << "\n";

    return 0;
}
