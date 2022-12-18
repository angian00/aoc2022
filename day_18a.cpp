#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <set>

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
        void add_voxel(GridPos3D voxel) {
            solid_voxels.insert(voxel);
        }

        int compute_surface() {
            int surface = 0;

            for (auto voxel: solid_voxels) {
                for (auto neighbor: enumerate_neighbors(voxel)) {
                    if (!solid_voxels.contains(neighbor))
                        surface ++;
                }
            }

            return surface;
        }

        void print() {
            cout << "# voxels: " << solid_voxels.size() << endl;

            for (auto voxel: solid_voxels) {
                cout << voxel << endl;
            }
        }

    private:
        static set<GridPos3D> enumerate_neighbors(GridPos3D pos) {
            set<GridPos3D> res;

            res.insert(GridPos3D(pos.x-1, pos.y,   pos.z  ));
            res.insert(GridPos3D(pos.x+1, pos.y,   pos.z  ));
            res.insert(GridPos3D(pos.x,   pos.y-1, pos.z  ));
            res.insert(GridPos3D(pos.x,   pos.y+1, pos.z  ));
            res.insert(GridPos3D(pos.x,   pos.y,   pos.z-1));
            res.insert(GridPos3D(pos.x,   pos.y,   pos.z+1));

            return res;
        }

        set<GridPos3D> solid_voxels;
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

    //droplet.print();

    cout << "total surface=" << droplet.compute_surface() << "\n";

    return 0;
}
