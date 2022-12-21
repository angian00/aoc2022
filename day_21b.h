#include <string>
#include <map>
#include <cassert>


using namespace std;

enum MonkeyType {
    Number,
    Operation,
    Human
};


class Monkey {
    public:
        static Monkey* parse(string s);

        Monkey(string name): name(name) {}
        string get_name() { return name; }
        
        virtual MonkeyType get_type() = 0;
        virtual long long get_value() = 0;
        virtual bool contains_human() = 0;


    protected:
        string name;
};


class HumanMonkey: public Monkey {
    public:
        HumanMonkey(): Monkey("humn") {}

        MonkeyType get_type() { return MonkeyType::Human; }
        long long get_value() {
            assert(false); //should never be called
            return -1;
        }

        bool contains_human() { return true; }

};

class NumberMonkey: public Monkey {
    public:
        NumberMonkey(string name, int number): Monkey(name), number(number) {}

        MonkeyType get_type() { return MonkeyType::Number; }
        long long get_value() { return number; }
        bool contains_human() { return false; }

    private:
        long long number;
};



enum OperatorType {
    Plus,
    Minus,
    Times,
    Divide,
    Equals
};

struct Operator {
    public:
        static Operator parse(char c);

        OperatorType get_type() { return type; }

        long long apply(long long operand1, long long operand2);
        Operator inverse();
        bool is_commutative();
        long long get_invariant();
        long long commutate(long long operand);

        friend bool operator==(const Operator& lhs, const Operator& rhs) {
            return ( lhs.type == rhs.type );
        }

        friend std::ostream & operator<<( std::ostream &os, const Operator &value );


    private:
        Operator(OperatorType op_type): type(op_type) {}
        
        OperatorType type;
};

class OperationMonkey: public Monkey {
    public:
        OperationMonkey(string name, string operand1_name, string operand2_name, Operator operatr): Monkey(name), 
            operand1_name(operand1_name), operand2_name(operand2_name), operatr(operatr) {}

        OperationMonkey(string name, Monkey* operand1, Monkey* operand2, Operator operatr): Monkey(name), 
            operand1(operand1), operand2(operand2), operatr(operatr) {}

        string get_operand1_name() { return operand1_name; }
        string get_operand2_name() { return operand2_name; }
        bool is_tree_computed() { return (operand1 != nullptr); }

        Monkey* get_operand1() { return operand1; }
        Monkey* get_operand2() { return operand2; }
        Operator get_operatr() { return operatr; }

        void set_operand1(Monkey *operand1) { this->operand1 = operand1; }
        void set_operand2(Monkey *operand2) { this->operand2 = operand2; }
        void set_operatr(Operator operatr) { this->operatr = operatr; }
        void set_operatr(char c) { this->operatr = Operator::parse(c); }

        void switch_branches();


        MonkeyType get_type() { return MonkeyType::Operation; }
        long long get_value();
        bool contains_human();


    private:
        string operand1_name;
        string operand2_name;
        Operator operatr;
        Monkey *operand1 = nullptr;
        Monkey *operand2 = nullptr;
};


class MonkeyTree {
    public:
        void add_monkey(Monkey* monkey) {
            monkey_map[monkey->get_name()] = monkey;
        }

        void build_tree();
        void print_tree();

        void solve();
        long long get_solution();

    private:
        void build_tree_from(Monkey*);
        void print_tree_from(Monkey*, int);
        void solve(Monkey*);
        void simplify(OperationMonkey*);

        map<string, Monkey*> monkey_map;
        OperationMonkey* root;
};

