#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <deque>
#include <regex>


using namespace std;


enum ParseState
{
	ParseStateStart,
	ParseStateBetween,
	ParseStateMoves
};


class Boxes
{
public:
	void parse_row(string line)
	{
		int i_stack = 0;
		int pos = 0;
		while (pos <= line.length() - 3) {
			if (i_stack >= stacks.size())
				stacks.push_back(deque<char>());

			auto token = line.substr(pos, 3);

			if (token != "   ")
				stacks[i_stack].push_back(token[1]);

			i_stack ++;
			pos += 4;
		}
	}


	void parse_move(string line)
	{
		//move 1 from 2 to 1
		static regex move_regex("move (\\d+) from (\\d+) to (\\d+)");
		smatch matches;

		regex_search(line, matches, move_regex);
		int n_boxes = stoi(matches.str(1));
		int stack_from = stoi(matches.str(2)) - 1;
		int stack_to = stoi(matches.str(3)) - 1;

		//cout << "moving # " << n_boxes << " boxes: " << " from: " << stack_from+1 << " to: " << stack_to+1 << endl;

		for (int i=0; i < n_boxes; i++) {
			auto& st = stacks[stack_from];
			char box = st[n_boxes-i-1];
			stacks[stack_to].push_front(box);
		}

		for (int i=0; i < n_boxes; i++)
			stacks[stack_from].pop_front();

		//print();
	}

	void print()
	{
		for (int i=0; i < stacks.size(); i++)
		{
			cout << "box stack #" << (i+1) << "  ";
			for (auto it=stacks[i].begin(); it != stacks[i].end(); it++)
			{
				cout << (*it) << " ";

			}
			cout << endl;
		}

		cout << endl;
	}

	void print_heads()
	{
		for (int i=0; i < stacks.size(); i++)
			cout << stacks[i][0];

		cout << endl;
	}

private:
	vector<deque<char>> stacks;	
};


int main(int argc, char* argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);


	Boxes boxes;
	ParseState parse_state = ParseStateStart;
	string line;

	while (getline(f, line))
	{
		switch (parse_state)
		{
			case ParseStateStart:
				if (line.substr(0, 3) == " 1 ")
					parse_state = ParseStateBetween;
				else
					boxes.parse_row(line);
				break;

			case ParseStateBetween:
				//boxes.print();
				parse_state = ParseStateMoves;
				break;

			case ParseStateMoves:
				boxes.parse_move(line);
				break;
		}
	}

	boxes.print_heads();

	return 0;
}

