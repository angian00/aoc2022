#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <regex>

#include "day_21a.h"


Monkey* Monkey::parse(string s) {
    static regex base_regex = regex("(.+): (.+)");
    static regex operation_regex = regex("(.+) (.) (.+)");
    smatch matches;

    bool is_match = regex_match(s, matches, base_regex);
    if (!is_match)
        throw std::invalid_argument("Malformed base monkey string: " + s);
    
    auto name = matches.str(1);
    auto expr = matches.str(2);
    
    is_match = regex_match(expr, matches, operation_regex);
    if (is_match) {
        string op1_name = matches.str(1);
        char operatr = matches.str(2)[0];
        string op2_name = matches.str(3);
        return new OperationMonkey(name, op1_name, op2_name, operatr);

    } else {
        return new NumberMonkey(name, stoi(expr));
    }
}



long long OperationMonkey::get_value() {
    if (operatr == '+')
        return operand1->get_value() + operand2->get_value();
    else if (operatr == '-')
        return operand1->get_value() - operand2->get_value();
    else if (operatr == '*')
        return operand1->get_value() * operand2->get_value();
    else if (operatr == '/')
        return operand1->get_value() / operand2->get_value();
    else
        throw std::invalid_argument("Unknown operator: " + operatr);
}



void MonkeyTree::build_tree() {
    build_tree_from(monkey_map["root"]);
}

void MonkeyTree::build_tree_from(Monkey* monkey) {
    if (monkey->get_type() != MonkeyType::Operation) 
        return;

    OperationMonkey* operation_monkey = (OperationMonkey*)monkey;
    if (operation_monkey->is_tree_computed()) 
        return;

    Monkey* operand1 = monkey_map[operation_monkey->get_operand1_name()];
    operation_monkey->set_operand1(operand1);
    build_tree_from(operand1);

    Monkey* operand2 = monkey_map[operation_monkey->get_operand2_name()];
    operation_monkey->set_operand2(operand2);
    build_tree_from(operand2);
}


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);


    MonkeyTree tree;
    while (getline(f, line))
    {
        tree.add_monkey(Monkey::parse(line));
    }

    clock_t t_start = clock();

    tree.build_tree();
    long long total = tree.get_root_value();

    clock_t t_end = clock();
    cout << "result: " << total << endl;
    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
