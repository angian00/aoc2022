#include <string>
#include <map>


using namespace std;

enum MonkeyType {
    Number,
    Operation
};


class Monkey {
    public:
        static Monkey* parse(string s);

        Monkey(string name): name(name) {}
        string get_name() { return name; }
        
        virtual MonkeyType get_type() = 0;
        virtual long long get_value() = 0;

    protected:
        string name;
};


class NumberMonkey: public Monkey {
    public:
        NumberMonkey(string name, int number): Monkey(name), number(number) {}

        MonkeyType get_type() { return MonkeyType::Number; }
        long long get_value() { return number; }

    private:
        long long number;
};


class OperationMonkey: public Monkey {
    public:
        OperationMonkey(string name, string operand1_name, string operand2_name, char operatr): Monkey(name), 
            operand1_name(operand1_name), operand2_name(operand2_name), operatr(operatr) {}

        string get_operand1_name() { return operand1_name; }
        string get_operand2_name() { return operand2_name; }
        bool is_tree_computed() { return (operand1 != nullptr); }

        void set_operand1(Monkey *operand1) { this->operand1 = operand1; }
        void set_operand2(Monkey *operand2) { this->operand2 = operand2; }

        MonkeyType get_type() { return MonkeyType::Operation; }
        long long get_value();


    private:
        string operand1_name;
        string operand2_name;
        char operatr;
        Monkey *operand1 = nullptr;
        Monkey *operand2 = nullptr;
};


class MonkeyTree {
    public:
        void add_monkey(Monkey* monkey) {
            monkey_map[monkey->get_name()] = monkey;
        }

        void build_tree();

        long long get_root_value() {
            return monkey_map["root"]->get_value();
        }

    private:
        void build_tree_from(Monkey* monkey);

        map<string, Monkey*> monkey_map;
};

