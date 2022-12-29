#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <set>
#include <map>
#include <algorithm>
#include <functional>
#include <regex>
#include <ranges>
#include <assert.h>
#include <time.h>
#include <climits>


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


    private:
        bool compare_valves (string v1, string v2) { return (valves[v1]->flow_rate > valves[v2]->flow_rate); }
        
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



struct StateAndMoves {
    ValveSystemState state;
    vector<ValveMove> moves;
};


string find_lowest(set<string> open_set, map<string, int> f_scores) {
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
            flowing_valves = valve_system->get_flowing_valves();

            // cout << "---- valves with flow_rate > 0: " << endl;
            // for (auto v: flowing_valves)
            //     cout << v << " " << valve_system->get_valve(v)->flow_rate << endl;
            // cout << endl;

            paths = compute_paths(flowing_valves);
            print_paths();
        }

        void print_paths() {
            cout << "---- computed paths: " << endl;

            for (auto path: paths) {
                cout << "path from " << path.first.first << " to " << path.first.second <<endl;
                cout << " \t ";
                for (auto step: path.second) 
                    cout << step << ",";
                cout << endl;
            }

            cout << endl;

        }

        void print_steps(vector<string> path) {
            for (auto step: path)
                cout << "\tgo to " << step << endl;
        }


        void optimize(string start_valve) {
            cout << "optimize " << endl;
            StateAndMoves state_and_moves;

            state_and_moves.state.curr_pos = start_valve;
            state_and_moves.state.remaining_steps = n_steps;
            
            auto opt_result = optimize_recurse(state_and_moves, 0);

            cout << endl;
            cout << "tot_pressure: " << opt_result.state.tot_pressure << endl;
            cout << endl;
            cout << "open valves: [";
            for (auto v_name: opt_result.state.open_valves) {
                cout << v_name << ", ";
            }
            cout << "]" << endl;
            cout << endl;

            if (trace) {
                cout << "moves:" << endl;
                string curr_valve = start_valve;
                for (auto move: opt_result.moves) {
                    if (move.move_type == MoveType::Move) {
                        cout << "\t(from " << curr_valve << " to " << move.target_valve << ")" << endl;
                        auto path = compute_path(curr_valve, move.target_valve);
                        print_steps(path);

                    } else
                        cout << "open " << move.target_valve << endl;
                    
                    curr_valve = move.target_valve;
                }
                cout << endl;
            }

            cout << "n_explored_nodes: " << n_explored_nodes << endl;
            cout << "n_leaves: " << n_leaves << endl;
            cout << "n_pressure_bound: " << n_pressure_bound << endl;
        }


        StateAndMoves optimize_recurse(StateAndMoves start_state, int best_so_far) {
            auto start_system_state = start_state.state;
            int depth = n_steps - start_system_state.remaining_steps;
            
            n_explored_nodes ++;
            if (start_system_state.remaining_steps == 0) {
                //cout << "optimize_recurse; base case; tot_pressure=" << start_state.state.tot_pressure << endl;
                n_leaves ++;
                return start_state;
            }

            assert(start_system_state.remaining_steps > 0);


            if (all_valves_open(start_state.state)) {
                // cout << "pressure so far: " << start_system_state.tot_pressure << 
                //     " remaining steps: " << start_system_state.remaining_steps << 
                //     " max flow: " <<  valve_system->get_max_flow() << endl;
                //start_state.state.tot_pressure += valve_system->get_max_flow() * start_system_state.remaining_steps;
                cout << "final pressure: " << start_state.state.tot_pressure << endl;
                n_leaves ++;
                return start_state;
            }

            // if (start_state.moves.size() >= 6) {
            //     cout << "tot_pressure: " << start_state.state.tot_pressure << " n_moves: " << start_state.moves.size() << " optimize_recurse" << endl;
            //     for (auto move: start_state.moves) {
            //         cout << move << endl;
            //     }

            //     //return start_state;
            // }

            //if (curr_state.remaining_steps >= 23)
            //    cout << curr_state.remaining_steps << endl;
            
            StateAndMoves best_result(start_state);
            //cout << "initial best_state: " << best_state << endl;

            for (auto move: enumerate_moves(start_state)) {
                ValveSystemState next_system_state = evolve_state(start_system_state, move);
                if (next_system_state.remaining_steps < 0) {
                    //cout << "negative remaining steps" << endl;
                    continue;
                }

                StateAndMoves state_and_moves(start_state);
                state_and_moves.state = next_system_state;
                state_and_moves.moves.push_back(move);

                //if (pressure_upper_bound_1(next_system_state) <= best_state.state.tot_pressure) {
                if (pressure_upper_bound_2(next_system_state) <= best_so_far) {
                     n_pressure_bound ++;
                     continue;
                }

                auto opt_result = optimize_recurse(state_and_moves, best_so_far);

                if (opt_result.state.tot_pressure > best_so_far && opt_result.state.tot_pressure > best_result.state.tot_pressure) {
                    // if (opt_result.state.tot_pressure > 1300) {
                    //     cout << "depth: " << depth << " move " << move << " improves tot_pressure to: " 
                    //         << opt_result.state.tot_pressure << endl;
                    // }
                    best_result = opt_result;
                    best_so_far = opt_result.state.tot_pressure;
                }
            }

            return best_result;
        }


    private:
        int pressure_upper_bound_1(ValveSystemState system_state) {
            int res = system_state.tot_pressure;
            string curr_pos = system_state.curr_pos;

            if (valve_system->get_valve(curr_pos)->flow_rate == 0) {
                //upper bound cannot be effectively computed
                return valve_system->get_max_flow() * system_state.remaining_steps;
            }

            for (auto v_name: valve_system->get_valves()) {
                if (!system_state.open_valves.contains(v_name)) {
                    int max_flowing_rounds = system_state.remaining_steps - 1;
                    res += max_flowing_rounds * (valve_system->get_valve(v_name)->flow_rate);
                }
            }

            return res;
        }

        int pressure_upper_bound_2(ValveSystemState system_state) {
            int res = system_state.tot_pressure;
            string curr_pos = system_state.curr_pos;

            if (valve_system->get_valve(curr_pos)->flow_rate == 0) {
                //upper bound cannot be effectively computed
                return valve_system->get_max_flow() * system_state.remaining_steps;
            }

            for (auto v_name: valve_system->get_valves()) {
                if (!system_state.open_valves.contains(v_name)) {
                    int min_distance;

                    if (curr_pos == v_name)
                        min_distance = 0;
                    else
                        min_distance = paths[make_pair(curr_pos, v_name)].size();

                    int max_flowing_rounds = system_state.remaining_steps - min_distance - 1;
                    if (max_flowing_rounds < 0)
                        max_flowing_rounds = 0;

                    res += max_flowing_rounds * (valve_system->get_valve(v_name)->flow_rate);
                }
            }

            return res;
        }



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
            //bool trace = (from == "EZ" && to == "IF");
            bool trace = false;
            if (trace) cout << "computing path from " << from << " to " << to << endl;

            set<string> queue;

            map<string, string> came_from;
        
            map<string, int> dists;
            for (auto curr_node: valve_system->get_valves()) {
                dists[curr_node] = INT_MAX;
                queue.insert(curr_node);
            }

            dists[from] = 0;

            while (!queue.empty()) {
                string curr_node = find_lowest(queue, dists);
                if (trace) cout << "-- Processing node " << curr_node << endl;

                if (curr_node == to) {
                    vector<string> full_path = reconstruct_path(came_from, curr_node);
                    return full_path;
                }

                queue.erase(curr_node);

                auto neighbors = find_neighbors(curr_node);
                for (auto neighbor: neighbors) {
                    if (!queue.contains(neighbor))
                        continue;

                    int new_dist = dists[curr_node] + 1;
                    if (new_dist < dists[neighbor]) {
                        if (trace) cout << "-- Better neighbor " << neighbor << " from node " << curr_node << 
                                " distance from " << dists[neighbor] << " to " << new_dist << endl;

                        came_from[neighbor] = curr_node;
                        dists[neighbor] = new_dist;
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


        vector<ValveMove> enumerate_moves(StateAndMoves from) {
            vector<ValveMove> res;
            
            string from_valve = from.state.curr_pos;
            bool is_from_flowing = (valve_system->get_valve(from_valve)->flow_rate > 0);
            bool is_from_open = from.state.open_valves.contains(from_valve);

            if (is_from_flowing && !is_from_open) {
                ValveMove valve_move(MoveType::Open, from_valve);
                //cout << "enumerate_moves a " << valve_move << endl;
                res.push_back(valve_move);

            }
            
            //moving twice in a row makes no sense
            if (from.moves.size() == 0 || from.moves[from.moves.size()-1].move_type != MoveType::Move) {
                for (auto maybe_to_valve: flowing_valves) {
                    //cout << "enumerate_moves b maybe_to_valve" << maybe_to_valve << endl;
                    if (maybe_to_valve != from_valve && !from.state.open_valves.contains(maybe_to_valve)) {
                        ValveMove valve_move(MoveType::Move, maybe_to_valve);
                        //cout << "enumerate_moves b ok " << valve_move << endl;
                        res.push_back(valve_move);
                    }
                }
            }

            return res;
        }

        ValveSystemState evolve_state(ValveSystemState start_state, ValveMove valve_move) {
            ValveSystemState res(start_state);
            //cout << "evolve_state " << endl;

            if (valve_move.move_type == MoveType::Open) {
                //cout << "evolve_state - MoveType::Open - remaining_steps=" << start_state.remaining_steps << endl;

                res.remaining_steps --;
                res.tot_pressure += valve_system->get_valve(valve_move.target_valve)->flow_rate * res.remaining_steps;
                //cout << "evolve_state - MoveType::Open - res.tot_pressure=" << res.tot_pressure<< endl;
                res.open_valves.insert(valve_move.target_valve);
            
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

                // cout << "evolve_state for move: " << valve_move << endl;

                // cout << "evolve_state - MoveType::Move - path:" << endl;
                // for (auto step: path) {
                //     cout << "\t" << step << endl;
                // }

                res.curr_pos = valve_move.target_valve;
                res.remaining_steps -= path.size();
            }


            return res;
        }


        bool all_valves_open(ValveSystemState system_state) {
            for (auto v_name: valve_system->get_flowing_valves()) {
                if (!system_state.open_valves.contains(v_name))
                    return false;
            }

            return true;
        }

        ValveSystem *valve_system;
        vector<ValveMove> opt_path;
        
        vector<string> flowing_valves;
        map<pair<string, string>, vector<string>> paths;

        long n_explored_nodes = 0;
        long n_leaves = 0;
        long n_pressure_bound = 0;
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

    //const int n_steps = 16;
    const int n_steps = 30;
    string start_valve = "AA";
    Optimizer* opt = new Optimizer(valve_system);
    
    clock_t t_start = clock();
    opt->optimize(start_valve); //1346 KO
    clock_t t_end = clock();
    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
