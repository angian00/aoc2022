#include <iostream>
#include <fstream>
#include <string>
#include <ctime>
#include <regex>

#include "day_21b.h"


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
        Operator operatr = Operator::parse(matches.str(2)[0]);
        string op2_name = matches.str(3);
        //cout << "creating monkey [" << name << "] as OperationMonkey" << endl;

        return new OperationMonkey(name, op1_name, op2_name, operatr);

    } else if (name == "humn") {
        //cout << "creating monkey [" << name << "] as HumanMonkey" << endl;
        return new HumanMonkey();
    } else {
        //cout << "creating monkey [" << name << "] as NumberMonkey" << endl;
        return new NumberMonkey(name, stoi(expr));
    }
}



long long OperationMonkey::get_value() {
    return operatr.apply(operand1->get_value(), operand2->get_value());
}

bool OperationMonkey::contains_human() {
    if (operand1->get_type() == MonkeyType::Human)
        return true;
    if (operand1->get_type() == MonkeyType::Operation && ((OperationMonkey *)operand1)->contains_human())
        return true;

    if (operand2->get_type() == MonkeyType::Human)
        return true;
    if (operand2->get_type() == MonkeyType::Operation && ((OperationMonkey *)operand2)->contains_human())
        return true;

    return false;
}

void OperationMonkey::switch_branches() {
    cout << name << " switch_branches" << endl;
    
    Monkey* tmp = operand1;
    operand1 = operand2;
    operand2 = tmp;
}



Operator Operator::parse(char c) {
    switch (c) {
        case '+':
            return Operator(OperatorType::Plus);
        case '-':
            return Operator(OperatorType::Minus);
        case '*':
            return Operator(OperatorType::Times);
        case '/':
            return Operator(OperatorType::Divide);
        case '=':
            return Operator(OperatorType::Equals);

        default:
            throw std::invalid_argument("Invalid operator: " + c);
    }
}

bool Operator::is_commutative() {
    return (type != OperatorType::Minus && type != OperatorType::Divide);
}

Operator Operator::inverse() {
    switch (type) {
        case OperatorType::Plus:
            return OperatorType::Minus;
        case OperatorType::Minus:
            return OperatorType::Plus;
        case OperatorType::Times:
            return OperatorType::Divide;
        case OperatorType::Divide:
            return OperatorType::Times;

        case OperatorType::Equals:
            return OperatorType::Equals;
    
        default:
            throw std::invalid_argument("Unmanaged operator");
    }
}

std::ostream & operator<<(std::ostream &os, const Operator &value)
{
    char c;

    switch (value.type) {
        case OperatorType::Plus:
            c = '+';
            break;
        case OperatorType::Minus:
            c = '-';
            break;
        case OperatorType::Times:
            c = '*';
            break;
        case OperatorType::Divide:
            c = '/';
            break;
        case OperatorType::Equals:
            c = '=';
            break;
    }

    os << c;

    return os;
}


long long Operator::get_invariant() {
    switch (type) {
        case OperatorType::Plus:
        case OperatorType::Minus:
            return 0LL;
        case OperatorType::Times:
        case OperatorType::Divide:
            return 1LL;

        case OperatorType::Equals:
            assert(false); //should never happen
    
        default:
            throw std::invalid_argument("Unmanaged operator");
    }
}

long long Operator::commutate(long long operand) {
    switch (type) {
        case OperatorType::Plus:
            return operand;
        case OperatorType::Minus:
            return -operand;
        case OperatorType::Times:
            return operand;
        case OperatorType::Divide:
            return 1/operand;

        case OperatorType::Equals:
            return OperatorType::Equals;
    
        default:
            throw std::invalid_argument("Unmanaged operator");
    }
}

long long Operator::apply(long long operand1, long long operand2) {
    switch (type) {
        case OperatorType::Plus:
            return operand1 + operand2;
        case OperatorType::Minus:
            return operand1 - operand2;
        case OperatorType::Times:
            return operand1 * operand2;
        case OperatorType::Divide:
            return operand1 / operand2;

        case OperatorType::Equals:
            assert(false); //should never happen

        default:
            throw std::invalid_argument("Unmanaged operator");
    }
}


void MonkeyTree::build_tree() {
    root = (OperationMonkey *)monkey_map["root"];
    build_tree_from(root);
    root->set_operatr('=');
}

void MonkeyTree::build_tree_from(Monkey* monkey) {
    //cout << "build_tree_from " << monkey->get_name() << endl;

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


long long MonkeyTree::get_solution() {
    solve(root);

    return root->get_operand2()->get_value();
}

void MonkeyTree::solve() {
    //cout << "solve" << endl;
    solve(root);
}

void MonkeyTree::solve(Monkey* monkey) {
    //cout << "solve " << monkey->get_name() << endl;

    if (monkey->get_type() != MonkeyType::Operation)
        return;


    OperationMonkey* start_node = (OperationMonkey*)monkey;
    
    if (start_node->get_operand1()->get_type() == MonkeyType::Human)
        return;

    if (start_node == root && !start_node->get_operand1()->contains_human()) {
        start_node->switch_branches();
    }
    
    simplify(start_node);
    //print_tree();
    //cout << "going to solve " << start_node->get_operand1()->get_name() << " as child of " << start_node->get_name() << endl;

    //solve(start_node->get_operand1());
    solve(start_node);
}


void MonkeyTree::simplify(OperationMonkey* start_node) {
    //cout << "simplify " << start_node->get_name() << endl;
    auto target_node = (OperationMonkey*)start_node->get_operand1();

    if (target_node->get_type() != MonkeyType::Operation)
        return;

    if (target_node->get_operand1()->contains_human()) {
        //pull up left branch, move right branch to the right
        //cout << "pulling right " << target_node->get_operand1()->get_name() << endl;
        start_node->set_operand1(target_node->get_operand1());

        Monkey* root_right_branch = root->get_operand2();
        //cout << "moving right " << target_node->get_operand2()->get_name() << endl;
        OperationMonkey* right_branch_new = new OperationMonkey(
            "_nc_", root_right_branch, target_node->get_operand2(), target_node->get_operatr().inverse());
        root->set_operand2(right_branch_new);

        free(target_node);

    } else {
        //pull up right branch, move left branch to the absolute right
        //cout << "before moving right" << endl;
        //print_tree();

        Monkey* root_right_branch = root->get_operand2();
        //cout << "moving right " << target_node->get_operand1()->get_name() << endl;
        Operator new_op = target_node->get_operatr();
        OperationMonkey* right_branch_new;
        if (new_op.is_commutative()) {
            new_op = new_op.inverse();
            right_branch_new = new OperationMonkey("_cd_", root_right_branch, target_node->get_operand1(), new_op);
        } else {
            right_branch_new = new OperationMonkey("_cc_", target_node->get_operand1(), root_right_branch, new_op);
        }

        root->set_operand2(right_branch_new);

        //cout << "before pulling up and left" << endl;
        //print_tree();
        //cout << "pulling up and left " << target_node->get_operand2()->get_name() << endl;
        start_node->set_operand1(target_node->get_operand2());
        //print_tree();
        free(target_node);
    }
}


long long solve_file(string filename) {
    cout << "processing file: " << filename << endl;

    const string input_dir = "input";
    ifstream f(input_dir + "/" + filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);


    MonkeyTree tree;
    while (getline(f, line))
    {
        tree.add_monkey(Monkey::parse(line));
    }

    //cout << "after file parsing" << endl;

    tree.build_tree();
    //cout << "before solve" << endl;
    //tree.print_tree();

    tree.solve();

    long long sol_value = tree.get_solution();
    //tree.print_tree();
    //cout << "solution: " << sol_value << endl;

    return sol_value;
}

void MonkeyTree::print_tree() {
    cout << "-------------------------------------------" << endl;
    print_tree_from(root, 0);
    cout << endl;
}

void MonkeyTree::print_tree_from(Monkey* monkey, int depth) {
    for (int i=0; i < depth; i++) {
        cout << "  ";
    }
    cout << monkey->get_name() << " ";

    if (monkey->get_type() == MonkeyType::Number)
        cout << monkey->get_value() << endl;
    else if (monkey->get_type() == MonkeyType::Human)
        cout << " H" << endl;
    else {
        OperationMonkey* node = (OperationMonkey*) monkey;
        cout << "[" << node->get_operatr() << "]" << endl;
        print_tree_from(node->get_operand1(), depth+1);
        print_tree_from(node->get_operand2(), depth+1);

    }

}


int main(int argc, char *argv[])
{
    /*
    assert(solve_file("test_day21_1.txt") == 3);
    assert(solve_file("test_day21_2.txt") == 3);
    assert(solve_file("test_day21_3.txt") == 1);
    assert(solve_file("test_day21_4.txt") == 1);
    assert(solve_file("test_day21_5.txt") == 1);
    assert(solve_file("test_day21_6.txt") == 2);
    assert(solve_file("test_day21_7.txt") == -1);
    assert(solve_file("test_day21_8.txt") == 900);
    assert(solve_file("test_day21_9.txt") == 5);
    assert(solve_file("test_day21_10.txt") == 5);
*/

    clock_t t_start = clock();

    long long total = solve_file(argv[1]);
    //long long total = 0;

    clock_t t_end = clock();
    cout << "result: " << total << endl;
    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}

