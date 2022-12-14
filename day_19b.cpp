#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <regex>
#include <map>
#include <time.h>


using namespace std;


//const int n_turns = 31;
const int n_turns = 32;


// enum order matters, since loops iterate over Geode first
// and hence can be more efficient 
enum ResourceType {
    Geode = 0,
    Obsidian,
    Clay,
    Ore,
    None,
    Unknown
};

const int N_RESOURCE_TYPES = 5;


ResourceType parse_resource(string s) {
    if (s == "none")
        return ResourceType::None;
    if (s == "ore")
        return ResourceType::Ore;
    if (s == "clay")
        return ResourceType::Clay;
    if (s == "obsidian")
        return ResourceType::Obsidian;
    if (s == "geode")
        return ResourceType::Geode;

    throw std::invalid_argument("Unknown resource type: " + s);
}

string resource_str(ResourceType r) {
    if (r == ResourceType::None)
        return "none";
    if (r == ResourceType::Ore)
        return "ore";
    if (r == ResourceType::Clay)
        return "clay";
    if (r == ResourceType::Obsidian)
        return "obsidian";
    if (r == ResourceType::Geode)
        return "geode";
    
    return "<unknown resource>";
}

string resource_str(int r) {
    return resource_str((ResourceType)r);
}


class Blueprint {
    public:
        Blueprint() {
            for (int robot_type=0; robot_type < N_RESOURCE_TYPES; robot_type++) {
                for (int cost_type=0; cost_type < N_RESOURCE_TYPES; cost_type++) {
                    robot_costs[robot_type][cost_type] = 0;
                }
            }
        }

        static Blueprint parse(string s) {
            Blueprint res;

            //example: " Each obsidian robot costs 3 ore and 14 clay."
            static regex parse_regex = regex(" Each (.+?) robot costs (\\d+) ([a-z]+)( and (\\d+) ([a-z]+))?\\.");
            
            for(auto regex_it = std::sregex_iterator(s.begin(), s.end(), parse_regex);
                        regex_it != std::sregex_iterator();
                        regex_it++)
            {
                std::smatch m = *regex_it;
                auto robot_type = parse_resource(m.str(1));
    
                int cost_value = stoi(m.str(2));
                ResourceType cost_type = parse_resource(m.str(3));
                res.robot_costs[(ResourceType)robot_type][(ResourceType)cost_type] = cost_value;
    
                if (m.str(4) != "") {
                    cost_value = stoi(m.str(5));
                    cost_type = parse_resource(m.str(6));
                    res.robot_costs[(ResourceType)robot_type][(ResourceType)cost_type] = cost_value;
                }
            }

            return res;
        }

        void print() {
            for (int robot_type=0; robot_type < N_RESOURCE_TYPES; robot_type++) {
                cout << resource_str((ResourceType)robot_type) << " robot" << endl;
                for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                    cout << "\t" << resource_str(resource_type) << " " << robot_costs[robot_type][resource_type] << endl;
                }
                cout << endl;
            }
        }

        int robot_costs[N_RESOURCE_TYPES][N_RESOURCE_TYPES];
};

struct OptimizationResult {
    public:
        OptimizationResult(int n_geodes, ResourceType choices[]) : n_geodes(n_geodes) {
            for (int i_turn=0; i_turn < n_turns; i_turn++) {
                this->choices[i_turn] = choices[i_turn];
            }
        }

        int n_geodes;
        ResourceType choices[n_turns];
};


class Optimizer {
    public:
        Optimizer(Blueprint blueprint): blueprint(blueprint) {}


        OptimizationResult optimize() {
            int start_stocks[N_RESOURCE_TYPES];
            int start_robots[N_RESOURCE_TYPES];
            ResourceType start_choices[n_turns];

            int ore_ore_cost = blueprint.robot_costs[ResourceType::Ore][ResourceType::Ore];
            int ore_clay_cost = blueprint.robot_costs[ResourceType::Clay][ResourceType::Ore];
            int initial_wait = ore_ore_cost < ore_clay_cost ? ore_ore_cost : ore_clay_cost;

            start_stocks[ResourceType::None] = 0;
            start_stocks[ResourceType::Ore] = initial_wait;
            start_stocks[ResourceType::Clay] = 0;
            start_stocks[ResourceType::Obsidian] = 0;
            start_stocks[ResourceType::Geode] = 0;

            start_robots[ResourceType::None] = 0;
            start_robots[ResourceType::Ore] = 1;
            start_robots[ResourceType::Clay] = 0;
            start_robots[ResourceType::Obsidian] = 0;
            start_robots[ResourceType::Geode] = 0;

            for (int i_turn=0; i_turn < n_turns; i_turn++) {
                if (i_turn < initial_wait)
                    start_choices[i_turn] = ResourceType::None;
                else
                    start_choices[i_turn] = ResourceType::Unknown;
            }

            auto opt_result = optimize_recurse(0, n_turns-initial_wait, start_stocks, start_robots, start_choices);
            // cout << "Best strategy: " << endl;
            // for (auto choice: opt_result.choices) {
            //     cout << "\t" << resource_str(choice) << endl; 
            // }

            cout << "n_nodes: " << n_explored_nodes << endl;
            cout << "n_leaves: " << n_leaves << endl;
            cout << "n_pruned_bounds: " << n_pruned_bounds << endl;

            return opt_result;
        }

    private:

        OptimizationResult optimize_recurse(int best_so_far, int remaining_turns, int start_stocks[], int start_robots[], ResourceType start_choices[]) {
            int depth = n_turns - remaining_turns;
            ResourceType best_choices[n_turns];


            n_explored_nodes ++;

            if (remaining_turns == 0) {
                n_leaves ++;
                return OptimizationResult(start_stocks[ResourceType::Geode], start_choices);
            }

            if (depth < 7)
                cout << "optimize_recurse; depth=" << depth << "; remaining_turns=" << remaining_turns << endl;


            //bool trace_recursion = (depth == 2 && start_choices[2] == ResourceType::Clay);  
            //bool trace_recursion = (depth == 2);  
            bool trace_recursion = false;
            
            if (trace_recursion) {
                cout << "optimize_recurse; depth=" << depth << endl;
                cout << "start_choices:" << endl;
                for (int i_turn=0; i_turn < depth; i_turn++)
                    cout << "\t" << resource_str(start_choices[i_turn]) << endl;

                cout << "start_stocks:" << endl;
                for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                    cout << "\t" << resource_str(resource_type) << " " << start_stocks[resource_type] << endl;
                }
                cout << "start_robots:" << endl;
                for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                    cout << "\t" << resource_str(resource_type) << " " << start_robots[resource_type] << endl;
                }
                cout << "blueprint.robot_costs:" << endl;
                for (int robot_type=0; robot_type < N_RESOURCE_TYPES; robot_type++) {
                    for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                        cout << "\t" << resource_str(robot_type) << " " << resource_str(robot_type) << " " << 
                            blueprint.robot_costs[robot_type][resource_type] << endl;
                    }
                }
                cout << endl;
            }

            for (int i_turn=0; i_turn < depth; i_turn++)
                best_choices[i_turn] = start_choices[i_turn];
                
            for (int new_robot_type=0; new_robot_type < N_RESOURCE_TYPES; new_robot_type++) {
                int stocks[N_RESOURCE_TYPES];
                int robots[N_RESOURCE_TYPES];
                ResourceType choices[n_turns];
                
                bool is_choice_valid = true;
                for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                    stocks[resource_type] = start_stocks[resource_type] - blueprint.robot_costs[new_robot_type][resource_type];
                    
                    if (stocks[resource_type] < 0) {
                        is_choice_valid = false;
                        break;
                    }
                }

                if (!is_choice_valid)
                    continue;
                
                for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                    stocks[resource_type] = stocks[resource_type] + start_robots[resource_type];
                    robots[resource_type] = start_robots[resource_type];
                }

                robots[new_robot_type] ++;

                if (is_choice_valid) {
                    //cout << "new choice: " << resource_str(new_robot_type) << endl;

                    //int at_most = upper_bound_1(remaining_turns-1, stocks[ResourceType::Geode], robots[ResourceType::Geode]);
                    int at_most = upper_bound_2(remaining_turns-1, stocks, robots);

                    //int at_most = upper_bound_3(remaining_turns-1, stocks, robots);
                    //int at_most = upper_bound_3(remaining_turns, stocks, robots);

                    if (at_most <= best_so_far) {
                        n_pruned_bounds ++;
                        is_choice_valid = false;
                    }
                }

                if (trace_recursion)
                    cout << "optimize_recurse; depth=" << depth << ";new_robot_type=" << resource_str(new_robot_type) 
                        << "; choice is " << (is_choice_valid ? "" : "not ") << "valid" << endl;

                if (!is_choice_valid)
                    continue;


                if (trace_recursion) {
                    cout << "stocks:" << endl;
                    for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                        cout << "\t" << resource_str(resource_type) << " " << stocks[resource_type] << endl;
                    }
                    cout << "robots:" << endl;
                    for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                        cout << "\t" << resource_str(resource_type) << " " << robots[resource_type] << endl;
                    }
                    cout << endl;
                }

                for (int i_turn=0; i_turn < depth; i_turn++) {
                    choices[i_turn] = start_choices[i_turn];
                }
                choices[depth] = (ResourceType)new_robot_type;


                if (stocks[ResourceType::Geode] > 0) {
                //     cout << "Found some geodes!" << endl;
                //     cout << "optimize_recurse; depth=" << depth << endl;
                //     cout << "stocks:" << endl;
                //     for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                //         cout << "\t" << resource_str(resource_type) << " " << stocks[resource_type] << endl;
                //     }
                //     cout << "robots:" << endl;
                //     for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                //         cout << "\t" << resource_str(resource_type) << " " << robots[resource_type] << endl;
                //     }

                //     cout << "choices:" << endl;
                //     for (int i_turn=0; i_turn <= depth; i_turn++) {
                //         cout << "\t" << i_turn << " " << resource_str(choices[i_turn]) << endl;
                //     }
                //     cout << endl;
                
                    //cout << ".";
                }


                auto rec_result = optimize_recurse(best_so_far, remaining_turns-1, stocks, robots, choices);
                if (rec_result.n_geodes > best_so_far) {
                    best_so_far = rec_result.n_geodes;
                    //cout << "improved solution: " << best_so_far << " depth=" << depth << ", choice=" 
                    //    << resource_str(rec_result.choices[depth]) << endl;

                    for (int i_turn=depth; i_turn < n_turns; i_turn++)
                        best_choices[i_turn] = rec_result.choices[i_turn];
                }
            }

            return OptimizationResult(best_so_far, best_choices);
        }

        int upper_bound_1(int remaining_turns, int start_geodes, int start_geode_robots) {
            return start_geodes + start_geode_robots * remaining_turns + remaining_turns * (remaining_turns-1) / 2;
        }

        int upper_bound_2(int remaining_turns, int start_stocks[], int start_robots[]) {
            int n_obs = start_stocks[ResourceType::Obsidian];
            int n_obs_robots = start_robots[ResourceType::Obsidian];
            int n_geodes = start_stocks[ResourceType::Geode];
            int n_geode_robots = start_robots[ResourceType::Geode];
            int geode_robot_cost = blueprint.robot_costs[ResourceType::Geode][ResourceType::Obsidian];

            for (int i_turn=0; i_turn < remaining_turns; i_turn++) {
                n_obs += n_obs_robots;
                n_geodes += n_geode_robots;
                if (n_obs >= geode_robot_cost) {
                    n_geode_robots ++;
                    n_obs -= geode_robot_cost;
                } else {
                    n_obs_robots ++;
                }
            }

            return n_geodes;
        }


        int upper_bound_3(int remaining_turns, int start_stocks[], int start_robots[]) {
            bool trace_upper_bound = false;
            //bool trace_upper_bound = (start_robots[ResourceType::Clay] == 0 && remaining_turns == 20);

            if (trace_upper_bound) cout << "upper_bound_3; remaining_turns: " << remaining_turns << endl;

            int stocks[N_RESOURCE_TYPES];
            int robots[N_RESOURCE_TYPES];

            for (int resource_type=0; resource_type < N_RESOURCE_TYPES; resource_type++) {
                stocks[resource_type] = start_stocks[resource_type];
                robots[resource_type] = start_robots[resource_type];
            }

            for (int i_turn=0; i_turn < remaining_turns; i_turn++) {
                if (trace_upper_bound) {
                    cout << "turn " << i_turn << endl;
                    cout << "starting stock: " << endl;
                    for (int resource_type=0; resource_type < N_RESOURCE_TYPES-1; resource_type++) {               
                        cout << resource_str(resource_type) << " " << stocks[resource_type] << endl;     
                    }

                    cout << "starting robots: " << endl;
                    for (int resource_type=0; resource_type < N_RESOURCE_TYPES-1; resource_type++) {               
                        cout << resource_str(resource_type) << " " << robots[resource_type] << endl;     
                    }
                }

                int next_robot_type = ResourceType::None;
                for (int resource_type=0; resource_type < N_RESOURCE_TYPES-2; resource_type++) {                    
                    if (stocks[resource_type+1] >= blueprint.robot_costs[resource_type][resource_type+1]) {
                        next_robot_type = resource_type;
                        break;
                    }
                }

                for (int resource_type=0; resource_type < N_RESOURCE_TYPES-1; resource_type++) {                    
                    stocks[resource_type] += robots[resource_type];
                }

                if (next_robot_type != ResourceType::None) {
                    robots[next_robot_type] ++;
                    stocks[next_robot_type+1] -= blueprint.robot_costs[next_robot_type][next_robot_type+1];
                }

                if (trace_upper_bound) {
                    cout << endl;
                    cout << "new robot: " << resource_str(next_robot_type) << endl;
                    cout << endl;

                    cout << "ending stock: " << endl;
                    for (int resource_type=0; resource_type < N_RESOURCE_TYPES-1; resource_type++) {               
                        cout << resource_str(resource_type) << " " << stocks[resource_type] << endl;     
                    }

                    cout << "ending robots: " << endl;
                    for (int resource_type=0; resource_type < N_RESOURCE_TYPES-1; resource_type++) {               
                        cout << resource_str(resource_type) << " " << robots[resource_type] << endl;     
                    }

                    cout << endl;
                }
            }

            if (trace_upper_bound) cout << "final geodes: " << stocks[ResourceType::Geode] << endl << endl;

            return stocks[ResourceType::Geode];
        }


    Blueprint blueprint;
    unsigned long long n_explored_nodes = 0;
    unsigned long long n_leaves = 0;
    unsigned long long n_pruned_bounds = 0;
};


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);


    vector<Blueprint> blueprints;
    while (getline(f, line))
    {
        Blueprint b = Blueprint::parse(line);
        //b.print();

        blueprints.push_back(b);
        //break; //DEBUG
    }

    clock_t t_start = clock();
    int tot_quality_level = 1;
    int i_blueprint = 1;
    for (auto b: blueprints) {
        auto opt = Optimizer(b);
        int n_geodes = opt.optimize().n_geodes;            
        cout << "Blueprint #" << i_blueprint << ": " << n_geodes << "\n";
        tot_quality_level *= n_geodes;

        if (i_blueprint >= 3)
            break;
        i_blueprint ++;
    }

    // auto opt = Optimizer(blueprints[1]);
    // auto opt_result = opt.optimize();
    // cout << "Blueprint result: " << opt_result.n_geodes << "\n";
    // cout << "Best strategy: " << endl;
    // for (int i_choice=0; i_choice < n_turns; i_choice++) {
    //     cout << "\t" << resource_str(opt_result.choices[i_choice]) << endl; 
    // }


    clock_t t_end = clock();
    //cout << "result: " << tot_quality_level << endl;  //must be > 10064
    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
