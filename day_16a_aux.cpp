#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <regex>
#include <functional>


using namespace std;


const bool trace = false;
const int n_steps = 30;

class Valve {
    public:
        static Valve* parse(string s) {
            Valve *v = new Valve();

            static regex parse_regex("Valve (..) has flow rate=(\\d+); tunnels? leads? to valves? (.+)");
            smatch matches;
            regex_search(s, matches, parse_regex);

            v->name = matches.str(1);
            v->flow_rate = stoi(matches.str(2));
            string tunnels_str = matches.str(3);

            size_t start_index = 0;
            size_t sep_index;
            do {
                sep_index = tunnels_str.find(',', start_index);
                auto end_index = (sep_index == string::npos ? tunnels_str.length() : sep_index);
                string tunnel_name = tunnels_str.substr(start_index, end_index - start_index);
                v->tunnels.push_back(tunnel_name);
                start_index = sep_index + 2;
            } while (sep_index != string::npos);

            return v;
        }

        friend bool operator==(const Valve& lhs, const Valve& rhs) { 
            return (lhs.name == rhs.name);
        }

        friend std::ostream & operator<<( std::ostream &os, const Valve &valve )
        {
            os << "(" << valve.name << ": " << valve.flow_rate << " => ";
            for (auto tunnel: valve.tunnels) {
                os << tunnel << ", ";
            }

            os << ")";
            
            return os;
        }

        string name;
        int flow_rate;
        vector<string> tunnels;
};


class ValveSystem {
    public:
        void add_valve(Valve *v) {
            valves[v->name] = v;
        }

        const Valve* get_valve(string valve_name) {
            return valves[valve_name];
        }

        const vector<string> get_valves() {
            vector<string> res;

            res.reserve(valves.size());
            for (auto entry: valves)
                res.push_back(entry.first);

            return res;
        }

        vector<string> get_flowing_valves() {
            vector<string> res;

            for (auto v: valves) {
                if (v.second->flow_rate > 0)
                    res.push_back(v.first);
            }

            auto comp_func = std::bind(&ValveSystem::compare_valves, this, std::placeholders::_1, std::placeholders::_2);
            sort(res.begin(), res.end(), comp_func);

            return res;
        }


        int get_max_flow() {
            int res = 0;
            for (auto v: valves) {
                if (v.second->flow_rate > 0)
                    res += v.second->flow_rate;
            }

            return res;
        }


        void print() {
            cout << "---- valve system: " << endl;

            for (auto v: valves) {
                cout << *v.second << endl;
            }

            cout << endl;
        }

        void export_graph() {
            for (auto valve_entry: valves) {
                cout << "n " << valve_entry.first << endl;
            }
            
            for (auto valve_entry: valves) {
                auto v_name = valve_entry.first;
                auto valve = valve_entry.second;
                for (auto tunnel: valve->tunnels) {
                    int j_node = valve2index(tunnel);
                    //cout << "e " << i_node << " " << j_node << " " << i_edge << " 1" << endl;
                    cout << "e " << v_name << " " << tunnel << endl;
                }
            }
        }

    private:
        bool compare_valves (string v1, string v2) { return (valves[v1]->flow_rate > valves[v2]->flow_rate); }
        
        int valve2index(string v_name) {
            int i_valve=0;
            for (auto valve_entry: valves) {
                if (valve_entry.second->name == v_name)
                    return i_valve;

                i_valve ++;
            }

            return -1;
        }

        map<string, Valve*> valves;
};



int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);


    ValveSystem* valve_system = new ValveSystem();
    
    while (getline(f, line))
    {
        Valve* valve = Valve::parse(line);
        valve_system->add_valve(valve);
    }

    //valve_system->print();
    string start_valve = "AA";

    valve_system->export_graph();
    
    return 0;
}
