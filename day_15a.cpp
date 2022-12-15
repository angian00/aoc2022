#include <iostream>
#include <fstream>
#include <string>
#include <regex>

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


struct SensorBeacon {
    public:
        SensorBeacon(GridPos sensor, GridPos beacon)
         : sensor(sensor), beacon(beacon) {
            distance = manhattan_distance(sensor, beacon);
        }

        static int manhattan_distance(GridPos pos1, GridPos pos2) {
            return  abs(pos1.x - pos2.x) + abs(pos1.y - pos2.y);
        }

        GridPos sensor;
        GridPos beacon;
        int distance;
};


class SensorBeaconMap {
    public:
        void add_sensor_beacon(GridPos sensor, GridPos beacon) {
            SensorBeacon sensor_beacon(sensor, beacon);

            sensor_beacons.push_back(sensor_beacon);
        }

        void update_distances() {
            distance_max = 0;
            x_min = INT_MAX;
            x_max = INT_MIN;

            for (auto sensor_beacon: sensor_beacons) {
                if (sensor_beacon.distance > distance_max)
                    distance_max = sensor_beacon.distance;

                if (sensor_beacon.sensor.x > x_max)
                    x_max = sensor_beacon.sensor.x;

                if (sensor_beacon.sensor.x < x_min)
                    x_min = sensor_beacon.sensor.x;
            }
        }

        int get_x_min() {
            return x_min;
        }

        int get_x_max() {
            return x_max;
        }

        int get_distance_max() {
            return distance_max;
        }

        bool is_known_empty(GridPos pos) {
            for (auto sensor_beacon: sensor_beacons) {
                int d = SensorBeacon::manhattan_distance(sensor_beacon.sensor, pos);
                if (d < sensor_beacon.distance) {
                    //cout << pos << " is empty because of sensor " << sensor_beacon.sensor 
                    //     << ", distance=" << d << ", sensor distance=" << sensor_beacon.distance << endl;

                    return true;
                }

                if (d == sensor_beacon.distance && pos != sensor_beacon.beacon) {
                    return true;
                }
            }

            return false;
        }
    
    private:
        vector<SensorBeacon> sensor_beacons;
        int x_min;
        int x_max;
        int distance_max;
};


int main(int argc, char *argv[])
{
    const int scan_row = 2000000;
    //const int scan_row = 10;

    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    int total = 0;

    static regex line_regex("Sensor at x=(-?\\d+), y=(-?\\d+): closest beacon is at x=(-?\\d+), y=(-?\\d+)");
    
    SensorBeaconMap sensor_beacons;

    while (getline(f, line))
    {
		smatch matches;
        regex_search(line, matches, line_regex);
        int sensor_x = stoi(matches.str(1));
        int sensor_y = stoi(matches.str(2));
        int beacon_x = stoi(matches.str(3));
        int beacon_y = stoi(matches.str(4));

        GridPos sensor_pos(sensor_x, sensor_y);
        GridPos beacon_pos(beacon_x, beacon_y);
        sensor_beacons.add_sensor_beacon(sensor_pos, beacon_pos);

    }

    sensor_beacons.update_distances();
    int d = sensor_beacons.get_distance_max();

    int n_empty = 0;
    for (int x = sensor_beacons.get_x_min() - d - 1; x < sensor_beacons.get_x_max() + d + 1; x++) {
        GridPos pos(x, scan_row);
        if (sensor_beacons.is_known_empty(pos))
            n_empty ++;
    }

    cout << n_empty << "\n";

    return 0;
}
