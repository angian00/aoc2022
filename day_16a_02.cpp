#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <regex>
#include <ranges>
#include <assert.h>
#include <time.h>
#include <climits>


using namespace std;


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

        void print() {
            cout << endl;

            for (auto v: valves) {
                cout << *v.second << endl;
            }
        }


    private:
        map<string, Valve*> valves;
};


struct ValveSystemState {
    ValveSystemState() {
        curr_pos = "INVALID";
        tot_pressure = 0;
        remaining_steps = -1;
        //open_valves == empty
    }

    ValveSystemState(const ValveSystemState& src_state) {
        curr_pos = src_state.curr_pos;
        tot_pressure = src_state.tot_pressure;
        remaining_steps = src_state.remaining_steps;

        //cout << "ValveSystemState copy constructor" << endl;
        for (auto ov: src_state.open_valves) {
            open_valves.insert(ov);
        }
    }

    int pressure_upper_bound(ValveSystem* valve_system) {
        int res = tot_pressure;
        
        for (auto v_name: valve_system->get_valves()) {
            if (!open_valves.contains(v_name))
                res += (remaining_steps-1) * (valve_system->get_valve(v_name)->flow_rate);
        }

        return res;
    }


    friend bool operator<(const ValveSystemState& lhs, const ValveSystemState& rhs) { 
        if (lhs.remaining_steps < rhs.remaining_steps)
            return true;

        if (lhs.remaining_steps == rhs.remaining_steps) {
            if (lhs.curr_pos < rhs.curr_pos)
                return true;
            if (lhs.curr_pos == rhs.curr_pos) {
                if (lhs.tot_pressure < rhs.tot_pressure)
                    return true;
                if (lhs.tot_pressure == rhs.tot_pressure)
                    return (lhs.open_valves < rhs.open_valves);
            }
        }

        return false;
    }

    friend std::ostream & operator<<( std::ostream &os, const ValveSystemState &state )
    {
        os << "(curr_pos: " << state.curr_pos;
        os << ", tot_pressure: " << state.tot_pressure;
        os << ", remaining_steps: " << state.remaining_steps;
        os << ")";
        
        return os;
    }

    string curr_pos;
    set<string> open_valves;
    int tot_pressure;
    int remaining_steps;
};


enum MoveType {
    Move,
    Open
};

struct ValveMove {
    MoveType move_type;
    string target_valve;
    
    friend std::ostream & operator<<( std::ostream &os, const ValveMove &move )
    {
        if (move.move_type == MoveType::Open)
            os << "open " << move.target_valve;
        else
            os << "go to " << move.target_valve;
        
        return os;
    }

};



struct OptimizationState {
    ValveSystemState system_state;
    vector<ValveMove> move_list;
};


string find_lowest_f(set<string> open_set, map<string, int> f_scores) {
    //FIXME: optimize by changing f_score data structure
    int min_f_score = INT_MAX;
    string min_node;

    for (auto pos_it = open_set.begin(); pos_it != open_set.end(); pos_it++) {
        string curr_node = *pos_it;
        if (f_scores[curr_node] < min_f_score) {
            min_f_score = f_scores[curr_node];
            min_node = curr_node;
        }
    }

    return min_node;
}

vector<string> reconstruct_path(map<string, string> came_from, string end_node) {
    vector<string> reversed_path;

    string curr_node = end_node;
    while (came_from.contains(curr_node)) {
        //cout << "-- reconstruct_path: found node " << curr_pos << endl;
        reversed_path.push_back(curr_node);

        curr_node = came_from[curr_node];
    }

    vector<string> path;
    for (int i=0; i < reversed_path.size(); i++) 
        path.push_back(reversed_path[reversed_path.size()-i-1]);

    return path;
}


class Optimizer {
    public:
        Optimizer(ValveSystem *valve_system) : valve_system(valve_system) {
            for (auto v: valve_system->get_valves()) {
                if (valve_system->get_valve(v)->flow_rate > 0)
                    flowing_valves.push_back(v);
            }

            //TODO: order by decreasing flow rate
            
            cout << "valves with flow_rate > 0: " << endl;
            for (auto v: flowing_valves)
                cout << v << endl;
            cout << endl;

            paths = compute_paths(flowing_valves);
        }

        void print_paths() {
            for (auto path: paths) {
                //cout << "path from " << path.first.first << " to " << path.first.second <<endl;
                cout << " \t ";
                for (auto step: path.second)
                    cout << step << ",";
                cout << endl;
            }
        }

        void print_path(vector<string> path) {
            for (auto step: path)
                cout << "\tgo to " << step << endl;
        }


        void optimize(int n_steps, string start_valve) {
            cout << "optimize " << endl;
            OptimizationState start_state;

            start_state.system_state.curr_pos = start_valve;
            start_state.system_state.remaining_steps = n_steps;
            
            auto end_state = optimize_recurse(start_state);

            cout << endl;
            cout << "tot_pressure: " << end_state.system_state.tot_pressure << endl;
            cout << endl;
            cout << "open valves: [";
            for (auto v_name: end_state.system_state.open_valves) {
                cout << v_name << ", ";
            }
            cout << "]" << endl;
            cout << endl;

            cout << "moves:" <<endl;
            string curr_valve = start_valve;
            for (auto move: end_state.move_list) {
                if (move.move_type == MoveType::Move) {
                    cout << "\t(from " << curr_valve << " to " << move.target_valve << ")" << endl;
                    auto path = compute_path(curr_valve, move.target_valve);
                    print_path(path);

                } else
                    cout << "open " << move.target_valve << endl;
                
                curr_valve = move.target_valve;
            }
            cout << endl;
        }

        OptimizationState optimize_recurse(OptimizationState start_state) {
            auto start_system_state = start_state.system_state;
            int depth = 30 - start_system_state.remaining_steps;
            if (depth < 10)
                cout << "depth: " << depth << " optimize_recurse" << endl;

            if (start_system_state.remaining_steps <= 0) {
                //cout << "optimize_recurse; base case; tot_pressure=" << start_state.system_state.tot_pressure << endl;
                return start_state;

            }

            //if (curr_state.remaining_steps >= 23)
            //    cout << curr_state.remaining_steps << endl;
            
            OptimizationState best_state(start_state);
            //cout << "initial best_state: " << best_state << endl;

            for (auto move: enumerate_moves(start_system_state)) {
                // if (depth == 0)
                //     cout << "depth: " << depth << " processing move: " << move << endl;

                OptimizationState next_state(start_state);
                ValveSystemState next_system_state = evolve_state(start_system_state, move);
                //if (next_system_state.tot_pressure > 0)
                    //cout << "after evolve_state, tot_pressure=" << next_system_state.tot_pressure << endl;

                next_state.system_state = next_system_state;
                next_state.move_list.push_back(move);
                // if (depth == 0) {
                //     cout << "depth: " << depth << " move list before recursion: " << endl;
                //     for (auto p_move: next_state.move_list)
                //         cout << p_move << endl;
                // }

                if (next_state.system_state.pressure_upper_bound(valve_system) < best_state.system_state.tot_pressure) {
                     n_pressure_bound ++;
                     continue;
                }

                auto final_state = optimize_recurse(next_state);
                // if (depth == 0) {
                //     cout << "depth: " << depth << " final_state.move_list after recursion: " << endl;
                //     for (auto p_move: final_state.move_list)
                //         cout << p_move << endl;
                // }

                if (final_state.system_state.tot_pressure > best_state.system_state.tot_pressure) {
                    // if (final_state.system_state.tot_pressure > 1400) {
                    //     cout << "depth: " << depth << " move " << move << " improves tot_pressure to: " << final_state.system_state.tot_pressure << endl;
                    // }
                    best_state = final_state;
                }
            }

            // if (depth == 0) {
            //     cout << "depth: " << depth << " best_state.move_list: " << endl;
            //     for (auto p_move: best_state.move_list)
            //         cout << p_move << endl;
            // }

            return best_state;
        }


    private:
        map<pair<string, string>, vector<string>> compute_paths(vector<string> nodes) {
            map<pair<string, string>, vector<string>> res;

            for (auto from: nodes) {
                for (auto to: nodes) {
                    if (from == to)
                        continue;

                    res[make_pair(from, to)] = compute_path(from, to);
                }
            }
            return res;
        }

        vector<string> compute_path(string from, string to) {
            //cout << "computing path from " << from << " to " << to << endl;

            set<string> open_set;
            open_set.insert(from);

            map<string, string> came_from;
        
            map<string, int> g_score;
            map<string, int> f_score;
            for (auto curr_node: valve_system->get_valves()) {
                g_score[curr_node] = INT_MAX;
                f_score[curr_node] = INT_MAX;
            }

            g_score[from] = 0;
            f_score[from] = heuristic(from, to);

            while (!open_set.empty()) {
                string curr_node = find_lowest_f(open_set, f_score);
                //cout << "-- Processing node " << curr_node << endl;

                if (curr_node == to) {
                    vector<string> full_path = reconstruct_path(came_from, curr_node);
                    return full_path;
                }

                open_set.erase(curr_node);

                auto neighbors = find_neighbors(curr_node);
                for (auto neighbor: neighbors) {
                    int tentative_g_score = g_score[curr_node] + 1;
                    if (tentative_g_score < g_score[neighbor]) {
                        //cout << "-- Better neighbor " << neighbor << " for node " << curr_pos << 
                        //        " g_score from " << g_score[neighbor] << " to " << tentative_g_score << endl;

                        came_from[neighbor] = curr_node;
                        g_score[neighbor] = tentative_g_score;
                        f_score[neighbor] = heuristic(neighbor, to);
                        if (!open_set.contains(neighbor)) {
                            open_set.insert(neighbor);
                        }
                    }
                }
            }

            vector<string> dummy;
            return dummy;
        }


        int heuristic(string node, string to) {
            return node == to ? 0 : 1;
        }

        vector<string> find_neighbors(string node) {
            return valve_system->get_valve(node)->tunnels;
        }


        vector<ValveMove> enumerate_moves(ValveSystemState from_state) {
            vector<ValveMove> res;
            
            string from_valve = from_state.curr_pos;
            bool is_from_flowing = (valve_system->get_valve(from_valve)->flow_rate > 0);
            bool is_from_open = from_state.open_valves.contains(from_valve);

            if (is_from_flowing && !is_from_open) {
                ValveMove valve_move(MoveType::Open, from_valve);
                //cout << "enumerate_moves a " << valve_move << endl;
                res.push_back(valve_move);

            }
            
            //cout << "enumerate_moves b " << endl;
            for (auto maybe_to_valve: flowing_valves) {
                if (maybe_to_valve != from_valve && !from_state.open_valves.contains(maybe_to_valve)) {
                    ValveMove valve_move(MoveType::Move, maybe_to_valve);
                    //cout << "enumerate_moves b " << valve_move << endl;
                    res.push_back(valve_move);
                }
            }

            return res;
        }

        ValveSystemState evolve_state(ValveSystemState start_state, ValveMove valve_move) {
            ValveSystemState res(start_state);
            //cout << "evolve_state " << endl;

            if (valve_move.move_type == MoveType::Open) {
                //cout << "evolve_state - MoveType::Open - remaining_steps=" << start_state.remaining_steps << endl;

                if (start_state.remaining_steps > 1)
                    res.tot_pressure += valve_system->get_valve(valve_move.target_valve)->flow_rate * (start_state.remaining_steps - 1);
                //cout << "evolve_state - MoveType::Open - res.tot_pressure=" << res.tot_pressure<< endl;
                res.open_valves.insert(valve_move.target_valve);
                res.remaining_steps --;
            
            } else {
                vector<string> path;
                if (valve_system->get_valve(start_state.curr_pos)->flow_rate > 0) {
                    path = paths[make_pair(start_state.curr_pos, valve_move.target_valve)];
                } else {
                    //path between initial valve and first flowing valve is not pre-computed
                    path = compute_path(start_state.curr_pos, valve_move.target_valve);
                }

                if (path.size() == 0) {
                    cout << "!!! path is malformed between " << start_state.curr_pos << " and " << valve_move.target_valve << endl;
                }

                res.curr_pos = valve_move.target_valve;
                res.remaining_steps -= path.size();
            }


            return res;
        }

        ValveSystem *valve_system;
        vector<ValveMove> opt_path;
        
        vector<string> flowing_valves;
        map<pair<string, string>, vector<string>> paths;

        int n_eval_states = 0;
        int n_pressure_bound = 0;
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

    valve_system->print();

    //const int n_steps = 16;
    const int n_steps = 30;
    string start_valve = "AA";
    Optimizer* opt = new Optimizer(valve_system);
    
    clock_t t_start = clock();
    opt->optimize(n_steps, start_valve);
    clock_t t_end = clock();
    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
