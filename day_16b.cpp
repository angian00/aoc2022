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


//const int n_steps = 22;
const int n_steps = 26;

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
        bool compare_valves(string v1, string v2) { return (valves[v1]->flow_rate > valves[v2]->flow_rate); }
        
        map<string, Valve*> valves;
};


struct ValveSystemState {
    ValveSystemState() {
        tot_pressure = 0;
        remaining_steps = -1;

        curr_pos1 = "INVALID";
        curr_pos2 = "INVALID";
        
        curr_dest1 = "INVALID";
        curr_dest2 = "INVALID";

        //open_valves == empty
        //active_path1 == empty
        //active_path2 == empty
    }

    ValveSystemState(const ValveSystemState& src_state) {
        //cout << "ValveSystemState copy constructor" << endl;
        tot_pressure = src_state.tot_pressure;
        remaining_steps = src_state.remaining_steps;

        curr_pos1 = src_state.curr_pos1;
        curr_pos2 = src_state.curr_pos2;
        curr_dest1 = src_state.curr_dest1;
        curr_dest2 = src_state.curr_dest2;

        for (auto ov: src_state.open_valves) {
            open_valves.insert(ov);
        }

        for (auto s1: src_state.active_path1) {
            active_path1.push_back(s1);
        }

        for (auto s2: src_state.active_path2) {
            active_path2.push_back(s2);
        }
    }

    friend bool operator<(const ValveSystemState& lhs, const ValveSystemState& rhs) { 
        if (lhs.remaining_steps < rhs.remaining_steps)
            return true;

        if (lhs.remaining_steps == rhs.remaining_steps) {
            if (lhs.tot_pressure < rhs.tot_pressure)
                return true;
            if (lhs.tot_pressure == rhs.tot_pressure) {
                if (lhs.open_valves < rhs.open_valves)
                    return true;

                if (lhs.open_valves == rhs.open_valves) {
                    if (lhs.curr_pos1 < rhs.curr_pos1)
                        return true;
                    if (lhs.curr_pos1 == rhs.curr_pos1) {
                        if (lhs.curr_pos2 < rhs.curr_pos2)
                            return true;
                    }
                }
            }
        }

        return false;
    }

    friend std::ostream & operator<<( std::ostream &os, const ValveSystemState &state )
    {
        os << "(tot_pressure: " << state.tot_pressure;
        os << ", remaining_steps: " << state.remaining_steps;
        os << ", curr_pos1: " << state.curr_pos1;
        os << ", curr_pos2: " << state.curr_pos2;
        os << ")";
        
        return os;
    }

    int tot_pressure;
    int remaining_steps;
    set<string> open_valves;

    string curr_pos1;
    string curr_pos2;
    string curr_dest1;
    string curr_dest2;
    deque<string> active_path1;
    deque<string> active_path2;
};


enum MoveType {
    Move,
    Open,
    Moving,
    Idle,
};

struct ValveMove {
    MoveType move_type;
    string target_valve;
    
    friend std::ostream & operator<<( std::ostream &os, const ValveMove &move )
    {
        if (move.move_type == MoveType::Open)
            os << "open " << move.target_valve;
        else if (move.move_type == MoveType::Move)
            os << "go to " << move.target_valve;
        else if (move.move_type == MoveType::Moving)
            os << "moving to " << move.target_valve;
        else if (move.move_type == MoveType::Idle)
            os << "idle";
        
        return os;
    }

    friend bool operator<(const ValveMove& lhs, const ValveMove& rhs) { 
        if (lhs.move_type < rhs.move_type)
            return true;

        if (lhs.move_type == rhs.move_type) {
            if (lhs.target_valve < rhs.target_valve)
                return true;
            if (lhs.target_valve == rhs.target_valve) {
                    return true;
            }
        }

        return false;
    }

    friend bool operator==(const ValveMove& lhs, const ValveMove& rhs) { 
        return (lhs.move_type == rhs.move_type && lhs.target_valve == rhs.target_valve);
    }
};



struct StateAndMoves {
    ValveSystemState state;
    vector<ValveMove> moves1;
    vector<ValveMove> moves2;
};


string find_lowest(set<string> open_set, map<string, int> f_scores) {
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
            //print_paths();
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

        vector<string> expand_moves(vector<ValveMove> moves, string start_valve) {
            vector<string> res;

            string curr_valve = start_valve;

            for (auto move: moves) {
                if (move.move_type == MoveType::Move) {
                    auto path = compute_path(curr_valve, move.target_valve);
                    for (auto step: path) {
                        res.push_back("move to " + step + "  ");
                    }

                } else if (move.move_type == MoveType::Open) {
                    res.push_back("open " + move.target_valve + "    ");
                } else if (move.move_type == MoveType::Moving) {
                    res.push_back("moving to " + move.target_valve);
                } else if (move.move_type == MoveType::Idle) {
                    res.push_back("idle");
                } else {
                    res.push_back("???");
                }
                
                curr_valve = move.target_valve;
            }

            return res;
        }


        void optimize(string start_valve) {
            cout << "optimize " << endl;
            bool trace = true;
            
            StateAndMoves state_and_moves;

            state_and_moves.state.curr_pos1 = start_valve;
            state_and_moves.state.curr_pos2 = start_valve;
            state_and_moves.state.remaining_steps = n_steps;
            
            auto opt_result = optimize_recurse(state_and_moves, 0);

            cout << endl;
            cout << "n_steps: " << n_steps << endl;
            cout << "tot_pressure: " << opt_result.state.tot_pressure << endl;
            cout << endl;
            cout << "open valves: [";
            for (auto v_name: opt_result.state.open_valves) {
                cout << v_name << ", ";
            }
            cout << "]" << endl;
            cout << endl;

            if (trace) {
                auto steps1 = expand_moves(opt_result.moves1, start_valve);
                auto steps2 = expand_moves(opt_result.moves2, start_valve);

                cout << "steps:" << endl;
                for (int i_step=0; i_step < n_steps; i_step++) {
                    cout << "\t me: " << (i_step < steps1.size() ? steps1[i_step] : "-");
                    cout << "\t elephant: " << (i_step < steps2.size() ? steps2[i_step] : "-");
                    cout << endl;
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
            
            //bool trace = (depth <= 1);
            bool trace = false;
            if (trace) cout << "optimize_recurse; depth=" << depth << endl;

            n_explored_nodes ++;
            if (start_system_state.remaining_steps == 0) {
                if (trace) cout << "optimize_recurse; base case; tot_pressure=" << start_state.state.tot_pressure << endl;
                n_leaves ++;
                return start_state;
            }

            assert(start_system_state.remaining_steps > 0);


            if (all_valves_open(start_state.state)) {
                // cout << "pressure so far: " << start_system_state.tot_pressure << 
                //     " remaining steps: " << start_system_state.remaining_steps << 
                //     " max flow: " <<  valve_system->get_max_flow() << endl;
                //start_state.state.tot_pressure += valve_system->get_max_flow() * start_system_state.remaining_steps;
                //cout << "all_valves_open; final pressure: " << start_state.state.tot_pressure << endl;
                n_leaves ++;
                return start_state;
            }


            StateAndMoves best_result(start_state);

            for (auto move12: enumerate_moves(start_state)) {
                if (trace) cout << "processing move [" << move12.first << "], [" << move12.second << "]" << endl;

                ValveSystemState next_system_state = evolve_state(start_system_state, move12);
                assert (next_system_state.remaining_steps >= 0);

                StateAndMoves state_and_moves(start_state);
                state_and_moves.state = next_system_state;
                state_and_moves.moves1.push_back(move12.first);
                state_and_moves.moves2.push_back(move12.second);

                if (pressure_upper_bound(next_system_state) <= best_so_far) {
                     n_pressure_bound ++;
                     continue;
                }

                auto opt_result = optimize_recurse(state_and_moves, best_so_far);

                if (opt_result.state.tot_pressure > best_so_far && opt_result.state.tot_pressure > best_result.state.tot_pressure) {
                    if (trace && opt_result.state.tot_pressure > 1500) {
                        cout << "depth: " << depth << " move " << move12.first << ", " << move12.second << " improves tot_pressure to: " 
                            << opt_result.state.tot_pressure << endl;
                    }
                    best_result = opt_result;
                    best_so_far = opt_result.state.tot_pressure;
                }
            }

            return best_result;
        }


    private:
        int pressure_upper_bound(ValveSystemState state) {
            int res = state.tot_pressure;

            for (auto v_name: valve_system->get_flowing_valves()) {
                if (!state.open_valves.contains(v_name)) {
                    int min_distance;

                    auto d1 = compute_min_distance(state.curr_dest1, v_name);
                    auto d2 = compute_min_distance(state.curr_dest2, v_name);

                    if (d1 < d2)
                        min_distance = d1;
                    else
                        min_distance = d2;

                    int max_flowing_rounds = state.remaining_steps - min_distance - 1;
                    if (max_flowing_rounds < 0)
                        max_flowing_rounds = 0;

                    res += max_flowing_rounds * (valve_system->get_valve(v_name)->flow_rate);
                }
            }

            return res;
        }


        int compute_min_distance(string from, string to) {
            if (valve_system->get_valve(from)->flow_rate == 0)
                return 0;

            if (from == to)
                return 0;

            return paths[make_pair(from, to)].size();
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


        vector<pair<ValveMove, ValveMove>> enumerate_moves(StateAndMoves from) {
            vector<pair<ValveMove, ValveMove>> res;
            
            auto moves1 = enumerate_moves_single(from, true);
            auto moves2 = enumerate_moves_single(from, false);

            for (auto move1: moves1) {
                for (auto move2: moves2) {
                    if (move1.target_valve != move2.target_valve || move1.move_type == MoveType::Idle || move2.move_type == MoveType::Idle)
                        res.push_back(make_pair(move1, move2));
                }
            }

            //remove symmetrical first moves
            if (from.state.remaining_steps == n_steps)
                return remove_symmetrical(res);

            //cout << "enumerating #" << res.size() << " moves" << endl;
            return res;
        }


        vector<ValveMove> enumerate_moves_single(StateAndMoves from, bool is_first) {
            string from_valve;
            deque<string> active_path;
            vector<ValveMove> res;
            vector<ValveMove> past_moves;

            if (is_first) {
                from_valve = from.state.curr_pos1;
                active_path = from.state.active_path1;
                past_moves = from.moves1;
            } else {
                from_valve = from.state.curr_pos2;
                active_path = from.state.active_path2;
                past_moves = from.moves2;
            }

            if (active_path.size() > 0) {
                res.push_back(ValveMove(MoveType::Moving, active_path[active_path.size()-1]));
                return res;
            }

            bool is_from_flowing = (valve_system->get_valve(from_valve)->flow_rate > 0);
            bool is_from_open = from.state.open_valves.contains(from_valve);

            if (is_from_flowing && !is_from_open) {
                ValveMove valve_move(MoveType::Open, from_valve);
                //cout << "enumerate_moves a " << valve_move << endl;
                res.push_back(valve_move);

            }
            
            //moving twice in a row makes no sense
            if (past_moves.size() == 0 || past_moves[past_moves.size()-1].move_type == MoveType::Open) {
                for (auto maybe_to_valve: flowing_valves) {
                    //cout << "enumerate_moves b maybe_to_valve" << maybe_to_valve << endl;
                    if (maybe_to_valve != from_valve && !from.state.open_valves.contains(maybe_to_valve)) {
                        ValveMove valve_move(MoveType::Move, maybe_to_valve);
                        //cout << "enumerate_moves b ok " << valve_move << endl;
                        res.push_back(valve_move);
                    }
                }
            }

            if (from.state.open_valves.size() >= valve_system->get_flowing_valves().size() - 1) {
                res.push_back(ValveMove(MoveType::Idle));
            }

            return res;
        }


        vector<pair<ValveMove, ValveMove>> remove_symmetrical(vector<pair<ValveMove, ValveMove>> moves) {
            vector<pair<ValveMove, ValveMove>> res;

            for (auto move12: moves) {
                if (move12.first < move12.second || move12.first == move12.second)
                    res.push_back(move12);
            }

            return res;
        }

        ValveSystemState evolve_state(ValveSystemState start_state, pair<ValveMove, ValveMove> valve_moves) {
            ValveSystemState res(start_state);
            
            evolve_state_single(&res, valve_moves.first, true);
            evolve_state_single(&res, valve_moves.second, false);

            res.remaining_steps --;

            return res;
        }

        void evolve_state_single(ValveSystemState* state, ValveMove valve_move, bool is_first) {
            //cout << "evolve_state " << endl;

            string curr_pos;
            deque<string> active_path;

            if (is_first) {
                curr_pos = state->curr_pos1;
            } else {
                curr_pos = state->curr_pos2;

            }

            if (valve_move.move_type == MoveType::Open) {
                //cout << "evolve_state - MoveType::Open - remaining_steps=" << start_state.remaining_steps << endl;

                state->tot_pressure += valve_system->get_valve(valve_move.target_valve)->flow_rate * (state->remaining_steps-1);
                //cout << "evolve_state - MoveType::Open - res.tot_pressure=" << res.tot_pressure<< endl;
                state->open_valves.insert(valve_move.target_valve);
            
            } else if (valve_move.move_type == MoveType::Move) {
                vector<string> path;
                if (valve_system->get_valve(curr_pos)->flow_rate > 0) {
                    path = paths[make_pair(curr_pos, valve_move.target_valve)];
                } else {
                    //path between initial valve and first flowing valve is not pre-computed
                    path = compute_path(curr_pos, valve_move.target_valve);
                }

                if (path.size() == 0) {
                    cout << "!!! path is malformed between " << curr_pos << " and " << valve_move.target_valve << endl;
                }

                if (path.size() > 1) {
                    for (int i_step=1; i_step < path.size(); i_step++) {
                        active_path.push_back(path[i_step]);
                    }
                }
                
                if (is_first) {
                    state->active_path1 = active_path;
                    state->curr_dest1 = valve_move.target_valve;
                    state->curr_pos1 = path[0];
                } else {
                    state->active_path2 = active_path;
                    state->curr_dest2 = valve_move.target_valve;
                    state->curr_pos2 = path[0];
                }
                
            } else if (valve_move.move_type == MoveType::Moving) {
                if (is_first) {
                    state->curr_pos1 = state->active_path1.front();
                    state->active_path1.pop_front();
                } else {
                    state->curr_pos2 = state->active_path2.front();
                    state->active_path2.pop_front();
                }
            }

                // cout << "evolve_state for move: " << valve_move << endl;

                // cout << "evolve_state - MoveType::Move - path:" << endl;
                // for (auto step: path) {
                //     cout << "\t" << step << endl;
                // }

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
