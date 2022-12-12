#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <assert.h>

using namespace std;

const int n_turns = 10000;
//const int n_turns = 20;


class WorryValue
{
    public:
        WorryValue(int start_value=-1): start_value(start_value) {}

        void set_bases(vector<int> bases) {
            remainders.clear();
            for (auto base=bases.begin(); base != bases.end(); base++) {
                remainders.insert(pair<int, int>(*base, start_value % (*base)));
            }

        }

        bool is_divisible_by(int divisor) {
            return (remainders[divisor] == 0);
        }

        WorryValue operator+(int const val) {
            WorryValue res;

            for (auto base=remainders.begin(); base != remainders.end(); base++) {
                int divisor = base->first;
                res.remainders[divisor] = (remainders[divisor] + (val % divisor)) % divisor;
            }

            return res;
        }

        WorryValue operator*(int const val) {
            WorryValue res;

            for (auto base=remainders.begin(); base != remainders.end(); base++) {
                int divisor = base->first;
                res.remainders[divisor] = (remainders[divisor] * (val % divisor)) % divisor;
            }

            return res;
        }

        WorryValue operator*(WorryValue const &val) {
            WorryValue res;

            for (auto base=remainders.begin(); base != remainders.end(); base++) {
                int divisor = base->first;
                res.remainders[divisor] = (remainders[divisor] * val.remainders.at(divisor)) % divisor;
            }

            return res;
        }

        friend std::ostream & operator<<( std::ostream &os, const WorryValue &value )
        {
            os << "\n";
            for (auto base=value.remainders.begin(); base != value.remainders.end(); base++) {
                os << "\t" << base->first << " " << base->second << endl;
            }

            os << endl;
            
            return os;
        }

    private:
        int start_value;
        map<int, int> remainders;
};


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

        WorryValue eval(WorryValue old_value) {
            //cout << "eval " << operand1 << " " << operatr << " " << operand2 << endl;

            assert(operand1 == "old");

            if (operand2 == "old") {
                assert(operatr == '*');
                return old_value * old_value;
            } else {
                int val2 = stoi(operand2);
                if (operatr == '+')
                    return old_value + val2;
                else
                    //operator == '*'
                    return old_value * val2;
            }
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

        void enqueue_item(WorryValue item) {
            items.push_back(item);
        }

        bool has_items() {
            return (items.size() > 0);
        }

        WorryValue inspect_item(int* p_next_monkey) {
            n_inspects ++;

            WorryValue curr_item = items.front();
            items.pop_front();

            //cout << "Inspecting item: " << curr_item << endl;

            curr_item = op.eval(curr_item);
            //cout << "After eval item=" << curr_item << endl;

            if ((curr_item.is_divisible_by(branch_divisor))) {
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

        long long get_n_inspects() {
            return this->n_inspects;
        }

        void set_bases(vector<int> bases) {
            for(auto& item: items) {
                item.set_bases(bases);
            }
        }

        void print_items() {

            for(auto& item: items) {
                cout << item;
                cout << endl;
            }
         
            cout << endl;
        }

    private:
        deque<WorryValue> items;
        Operation op = {};
        int branch_divisor;
        int branch_true;
        int branch_false;
        long long n_inspects = 0;
};


vector<Monkey> parse_monkeys(string filename) {
    vector<Monkey> monkeys;
    vector<int> primes;

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
        primes.push_back(branch_divisor);

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

    for (auto monkey_it=monkeys.begin(); monkey_it != monkeys.end(); monkey_it++) {
        monkey_it->set_bases(primes);
    }

    return monkeys;
}


vector<Monkey> process_monkeys(vector<Monkey> monkeys, int n_turns) {
    for (int i_turn=0; i_turn < n_turns; i_turn++) {
        if ((i_turn+1) % 1000 == 0)
            cout << "Turn #" << (i_turn+1) << endl;

        int i_monkey = 0;
        for (auto monkey_it=monkeys.begin(); monkey_it != monkeys.end(); monkey_it++) {
            //cout << "Monkey #" << i_monkey << endl;
            while (monkey_it->has_items()) {
                int next_monkey;
                WorryValue curr_item = monkey_it->inspect_item(&next_monkey);
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

        if ((i_turn+1) % 1000 == 0) {
            for (auto i_monkey=0; i_monkey < monkeys.size(); i_monkey++) {
                cout << "Monkey " << i_monkey << " inspected items " << monkeys[i_monkey].get_n_inspects() << " times." << endl;
            }
        }
    }

    return monkeys;   
}

long long compute_monkey_business(vector<Monkey> monkeys) {
    long long max_inspects[2] = { 0, 0 };

    for (auto monkey_it=monkeys.begin(); monkey_it != monkeys.end(); monkey_it++) {
        long long curr_inspects = monkey_it->get_n_inspects();
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
