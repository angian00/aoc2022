#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <queue>
#include <assert.h>

using namespace std;


const int n_turns = 20;
const int relief_divisor = 3;


class Operation
{
    public:
        Operation() {
            operand1 = "";
            operand2 = "";
            operatr = '?';
        }

        static Operation from_expr(string expr) {
            //e. g. new = old + 8
            auto operatr_pos = expr.find('+');
            if (operatr_pos == string::npos)
                operatr_pos = expr.find('*');

            string operand1 = expr.substr(0, operatr_pos - 1);
            string operand2 = expr.substr(operatr_pos + 2);
            char operatr = expr[operatr_pos];

            return Operation(operand1, operand2, operatr);
        }

        int eval(int old_value) {
            //cout << "eval " << operand1 << " " << operatr << " " << operand2 << endl;

            int val1;
            if (operand1 == "old")
                val1 = old_value;
            else
                val1 = stoi(operand1);

            int val2;
            if (operand2 == "old")
                val2 = old_value;
            else
                val2 = stoi(operand2);
            
            if (operatr == '+')
                return val1 + val2;
            else
                //operator == '*'
                return val1 * val2;
        }


    private:
        Operation(string operand1, string operand2, char operatr): 
            operand1(operand1), operand2(operand2), operatr(operatr) {}

        string operand1;
        string operand2;
        char operatr;
};

class Monkey
{
    public:
        Monkey() = default;

        void enqueue_item(int item) {
            items.push(item);
        }

        bool has_items() {
            return (items.size() > 0);
        }

        int inspect_item(int* p_next_monkey) {
            n_inspects ++;

            int curr_item = items.front();
            items.pop();

            //cout << "Inspecting item: " << curr_item << endl;

            curr_item = op.eval(curr_item);
            //cout << "After eval item=" << curr_item << endl;

            curr_item /= relief_divisor;
            //cout << "After relief item=" << curr_item << endl;

            if ((curr_item % branch_divisor) == 0) {
                //cout << "branching true" << endl;
                (*p_next_monkey) = branch_true;
            } else {
                //cout << "branching false" << endl;
                (*p_next_monkey) = branch_false;
            }

            //cout << "Sending " << curr_item << " to monkey " << (*p_next_monkey) << endl;
            //cout << endl;

            return curr_item;
        }

        void set_op(Operation op) {
            this->op = op;
        }

        void set_branch(int branch_divisor, int branch_true, int branch_false) {
            //cout << "set_branch " << branch_divisor << endl;
            this->branch_divisor = branch_divisor;
            this->branch_true = branch_true;
            this->branch_false = branch_false;
        }

        int get_n_inspects() {
            return this->n_inspects;
        }

        void print_items() {
            queue<int> items_copy = items;

            while (!items_copy.empty()) {
                cout << items_copy.front() << ", ";
                items_copy.pop();
            }

            cout << endl;
        }

    private:
        queue<int> items;
        Operation op = {};
        int branch_divisor;
        int branch_true;
        int branch_false;
        int n_inspects = 0;
};


vector<Monkey> parse_monkeys(string filename) {
    vector<Monkey> monkeys;

    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    while (getline(f, line))
    {
        // -- for example
        // Monkey 0:
        //   Starting items: 79, 98
        //   Operation: new = old * 19
        //   Test: divisible by 23
        //     If true: throw to monkey 2
        //     If false: throw to monkey 3
        //
        // --

        assert(line.starts_with("Monkey "));
        Monkey new_monkey;

        getline(f, line);
        assert(line.starts_with("  Starting items: "));
        size_t sep_pos = line.find(":");
        

        string item_list = line;
        //cout << "item_list: " << item_list << endl;
        while (sep_pos != string::npos) {
            item_list = item_list.substr(sep_pos + 2);
            size_t sep_pos_next = item_list.find(",");

            string item_str = item_list.substr(0, (sep_pos_next == string::npos ? item_list.length() : sep_pos_next));
            
            new_monkey.enqueue_item(stoi(item_str));
            //cout << "item: " << item_str << endl;

            sep_pos = sep_pos_next;
        }


        getline(f, line);
        assert(line.starts_with("  Operation: new = "));
        sep_pos = line.find("=");
        string expr = line.substr(sep_pos+2);

        new_monkey.set_op(Operation::from_expr(expr));


        getline(f, line);
        assert(line.starts_with("  Test: divisible by "));
        sep_pos = line.find("by ");
        int branch_divisor = stoi(line.substr(sep_pos+3));

        getline(f, line);
        assert(line.starts_with("    If true: throw to monkey "));
        int branch_true = line[line.length()-1] - '0'; //FIXME: works only with <= 10 monkeys

        getline(f, line);
        assert(line.starts_with("    If false: throw to monkey "));
        int branch_false = line[line.length()-1] - '0'; //FIXME: works only with <= 10 monkeys
        
        new_monkey.set_branch(branch_divisor, branch_true, branch_false);

        
        //empty line separates monkeys
        getline(f, line);

        monkeys.push_back(new_monkey);
        //break; //DEBUG
    }
    return monkeys;
}


vector<Monkey> process_monkeys(vector<Monkey> monkeys, int n_turns) {

    for (int i_turn=0; i_turn < n_turns; i_turn++) {
        //cout << "Turn #" << i_turn << endl;

        int i_monkey = 0;
        for (auto monkey_it=monkeys.begin(); monkey_it != monkeys.end(); monkey_it++) {
            //cout << "Monkey #" << i_monkey << endl;
            while (monkey_it->has_items()) {
                int next_monkey;
                int curr_item = monkey_it->inspect_item(&next_monkey);
                monkeys[next_monkey].enqueue_item(curr_item);
            }

            i_monkey++;
        }

        // i_monkey = 0;
        // for (auto monkey_it=monkeys.begin(); monkey_it != monkeys.end(); monkey_it++) {
        //     cout << "Monkey #" << i_monkey << ": ";
        //     monkey_it->print_items();
        //     i_monkey++;
        // }
    }

    for (auto i_monkey=0; i_monkey < monkeys.size(); i_monkey++) {
        cout << "Monkey " << i_monkey << " inspected items " << monkeys[i_monkey].get_n_inspects() << " times." << endl;
    }

    return monkeys;   
}

int compute_monkey_business(vector<Monkey> monkeys) {
    int max_inspects[2] = { 0, 0 };

    for (auto monkey_it=monkeys.begin(); monkey_it != monkeys.end(); monkey_it++) {
        int curr_inspects = monkey_it->get_n_inspects();
        if (curr_inspects > max_inspects[0]) {
            max_inspects[1] = max_inspects[0];
            max_inspects[0] = curr_inspects;

        } else if (curr_inspects > max_inspects[1]) {
            max_inspects[1] = curr_inspects;
        }
    }

    return max_inspects[0] * max_inspects[1];
}


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    vector<Monkey> monkeys = parse_monkeys(filename);

    monkeys = process_monkeys(monkeys, n_turns);

    //int total = 0;
    cout << compute_monkey_business(monkeys) << "\n";

    return 0;
}
