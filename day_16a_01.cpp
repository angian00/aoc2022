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


class Optimizer {
    public:
        Optimizer(ValveSystem *valve_system) : valve_system(valve_system) {}

        void optimize(int n_steps, string start_valve) {
            //initialize full path
            for (int i=0; i < n_steps; i++) {
                ValveMove dummy_move;
                opt_path.push_back(dummy_move);
            }

            ValveSystemState start_state;

            start_state.curr_pos = start_valve;
            start_state.remaining_steps = n_steps;
            ValveSystemState final_state = optimize_with_limited_recurse(start_state);

            cout << endl;
            cout << "tot_flow: " << final_state.tot_pressure << endl;
            cout << endl;
            cout << "open valves: [";
            for (auto v_name: final_state.open_valves) {
                cout << v_name << ", ";
            }
            cout << "]" << endl;
            cout << endl;
        }


        enum MoveType {
            Move,
            Open
        };


    private:
        const int min_compute_bounds = 1;
        const int max_recurse_depth = 9;

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

        struct ValveMove {
            MoveType move_type;
            string target_valve;

            friend std::ostream & operator<<( std::ostream &os, const ValveMove &v_move )
            {
                if (v_move.move_type == MoveType::Move)
                    os << "go to " << v_move.target_valve;
                else
                    os << "open " << v_move.target_valve;
                
                return os;
            }

        };


        ValveSystemState optimize_with_limited_recurse(ValveSystemState start_state) {
            ValveSystemState curr_state = start_state;

            while (curr_state.remaining_steps > 0) {
                //cout << "main loop; curr_state: " << curr_state << endl;

                int best_pressure = 0;
                ValveMove best_move;

                for (auto move: enumerate_moves(curr_state)) {
                    ValveSystemState next_state = evolve_state(curr_state, move);
                    // if ((next_state.pressure_upper_bound(valve_system)) < best_pressure) {
                    //     n_pressure_bound ++;
                    //     continue;
                    // }

                    auto rec_state = optimize_recurse(next_state, max_recurse_depth);
                    //cout << "rec_state.tot_pressure: " << rec_state.tot_pressure << " best_pressure: " << best_pressure << endl;

                    if (rec_state.tot_pressure > best_pressure) {
                        best_pressure = rec_state.tot_pressure;
                        best_move = move;
                        //cout << "assigning best move: " << best_move << ", best_pressure: " << best_pressure << endl;
                    }
                }

                cout << best_move << endl;
                curr_state = evolve_state(curr_state, best_move);
            }

            cout << "# eval states: " << n_eval_states << ", #pressure_upper_bound: " << n_pressure_bound << endl;
            return curr_state;
        }

        ValveSystemState optimize_recurse(ValveSystemState curr_state, int curr_recurse_depth) {
            //if (curr_state.remaining_steps > 12)
                //cout << "remaining steps: " << curr_state.remaining_steps << endl;

            n_eval_states ++;

            if (curr_state.remaining_steps == 0 || curr_recurse_depth == 0)
                return curr_state;

            ValveSystemState best_state;
            //cout << "initial best_state: " << best_state << endl;

            for (auto move: enumerate_moves(curr_state)) {
                ValveSystemState next_state = evolve_state(curr_state, move);
                // if (next_state.remaining_steps > min_compute_bounds && next_state.pressure_upper_bound(valve_system) < best_state.tot_pressure) {
                //     n_pressure_bound ++;
                //     continue;
                // }

                ValveSystemState final_state = optimize_recurse(next_state, curr_recurse_depth-1);
                if (final_state.tot_pressure > best_state.tot_pressure) {
                    best_state = final_state;
                }
            }

            return best_state;
        }



        pair<ValveSystemState, vector<ValveMove>> optimize_recurse_old(ValveSystemState curr_state, vector<ValveMove> moves_so_far) {
            //if (curr_state.remaining_steps > 12)
            //    cout << "remaining steps: " << curr_state.remaining_steps << endl;

            if (curr_state.remaining_steps == 0)
                return pair(curr_state, moves_so_far);

            ValveSystemState opt_state;
            vector<ValveMove> opt_moves;
            auto next_moves = vector<ValveMove>(moves_so_far);

            for (auto move: enumerate_moves(curr_state)) {
                ValveSystemState next_state = evolve_state(curr_state, move);
                //if (visited_states.contains(next_state))
                //    continue;
                
                //visited_states.insert(next_state);
                next_moves[curr_state.remaining_steps-1] = move;
                auto opt_result = optimize_recurse_old(next_state, next_moves);
                ValveSystemState final_state = opt_result.first;
                vector<ValveMove> all_moves = opt_result.second;

                if (final_state.tot_pressure > opt_state.tot_pressure) {
                    opt_state = final_state;
                    opt_moves = all_moves;
                }
            }

            return pair(opt_state, opt_moves);
        }

        vector<ValveMove> enumerate_moves(ValveSystemState start_state) {
            vector<ValveMove> res;
            
            if (!start_state.open_valves.contains(start_state.curr_pos)) {
                ValveMove valve_move(MoveType::Open, start_state.curr_pos);
                res.push_back(valve_move);
            }

            for (auto tunnel: valve_system->get_valve(start_state.curr_pos)->tunnels) {
                ValveMove valve_move(MoveType::Move, tunnel);
                res.push_back(valve_move);
            }

            return res;
        }

        ValveSystemState evolve_state(ValveSystemState start_state, ValveMove valve_move) {
            ValveSystemState res(start_state);

            if (valve_move.move_type == MoveType::Open) {
                if (start_state.remaining_steps > 1)
                    res.tot_pressure += valve_system->get_valve(valve_move.target_valve)->flow_rate * (start_state.remaining_steps - 1);
                res.open_valves.insert(valve_move.target_valve);
            } else {
                res.curr_pos = valve_move.target_valve;
            }

            res.remaining_steps --;

            return res;
        }

        ValveSystem *valve_system;
        vector<ValveMove> opt_path;
        //set<ValveSystemState> visited_states;
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
    
    string start_valve = "";
    while (getline(f, line))
    {
        Valve* valve = Valve::parse(line);
        if (start_valve == "")
            start_valve = valve->name;

        valve_system->add_valve(valve);
    }

    valve_system->print();

    //const int n_steps = 16;
    const int n_steps = 30;
    Optimizer* opt = new Optimizer(valve_system);
    
    clock_t t_start = clock();
    opt->optimize(n_steps, start_valve);
    clock_t t_end = clock();
    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
