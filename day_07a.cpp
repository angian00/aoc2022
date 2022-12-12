#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>

using namespace std;


const int max_dir_size = 100'000;


class File
{
    public:
        File(string n, int s): name(n), size(s) { }

        const string get_name() { return name; }
        const int get_size() { return size; }

    private:
        string name;
        int size;
};


class Directory
{
    public:
        Directory(string n, Directory *parent_dir = nullptr): name(n), parent_dir(parent_dir) { }

        const string get_name() { return name; }
        Directory* get_parent_dir() { return parent_dir; }

        int get_size() {
            int res = 0;
            for (auto it = files.begin(); it != files.end(); it ++)
                res += (*it)->get_size();

            for (auto it = subdirs.begin(); it != subdirs.end(); it ++)
                res += (*it)->get_size();

            return res;
        }

        Directory *make_subdir(string subdir_name) {
            if (subdir_name[0] == '/')
                throw std::system_error(errno, std::system_category(), "absoluted dir names not supported ");

            for (auto it = subdirs.begin(); it != subdirs.end(); it ++) {
                if ((*it)->get_name() == subdir_name)
                    return (*it);
            }

            Directory* p_new_subdir = new Directory(subdir_name, this);
            subdirs.push_back(p_new_subdir);
            
            return p_new_subdir;
        }

        File *make_file(string file_name, int file_size) {
            for (auto it = files.begin(); it != files.end(); it ++) {
                if ((*it)->get_name() == file_name)
                    return (*it);
            }

            File* p_new_file = new File(file_name, file_size);
            files.push_back(p_new_file);
            
            return p_new_file;
        }

        void print(int indent_level = 0) {
            for (int i=0; i < indent_level; i++)
                cout << "  ";
            cout << "- " << name << " (dir)" << endl;

            for (auto it = subdirs.begin(); it != subdirs.end(); it ++) {
                (*it)->print(indent_level+1);
            }

            for (auto it = files.begin(); it != files.end(); it ++) {
                for (int i=0; i < indent_level; i++)
                    cout << "  ";
                cout << "  - " << (*it)->get_name() << " (file, size=" << (*it)->get_size() << ")" << endl;
            }
        }

        const vector<Directory*> get_subdirs() {
            return subdirs;
        }



        int recurse_compute_total_size(int max_dir_size)
        {
            int total_size = 0;
        
            int dir_size = get_size();
            if (dir_size <= max_dir_size)
                total_size += dir_size;
            
            for (auto it = subdirs.begin(); it != subdirs.end(); it ++) {
                total_size += (*it)->recurse_compute_total_size(max_dir_size);
            }

            //cout << name << ": " << dir_size << ", " << total_size << "\n";

            return total_size;
        }


    private:
        string name;
        Directory* parent_dir;
        vector<File*> files;
        vector<Directory*> subdirs;
};


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    Directory* p_root_dir = new Directory("/");
    Directory* p_curr_dir;

    while (getline(f, line))
    {
        //p_curr_dir = parse_line(line, &root_dir, p_curr_dir);
        if (line.substr(0, 2) == "$ ") {
            string cmd = line.substr(2, line.length()-2);

            if (cmd == "ls") {
                //curr_parse_state = ParseStateLsOutputExpected;
            
            } else {
                int delim_pos = cmd.find(' ');
                string cmd_name = cmd.substr(0, delim_pos);
                assert(cmd_name == "cd");

                string target_dir = cmd.substr(delim_pos+1, cmd.length() - delim_pos-1);

                if (target_dir == "/")
                    p_curr_dir = p_root_dir;
                else if (target_dir == "..")
                    p_curr_dir = p_curr_dir->get_parent_dir();
                else
                    p_curr_dir = p_curr_dir->make_subdir(target_dir);

                //curr_parse_state = ParseStateCdOutputExpected;
            }

        } else {
            //ParseStateCdOutputExpected: ls output line
            int delim_pos = line.find(' ');
            string token1 = line.substr(0, delim_pos);
            string token2 = line.substr(delim_pos+1, line.length() - delim_pos-1);

            if (token1 == "dir")
                p_curr_dir->make_subdir(token2);
            else {
                int file_size = stoi(token1);
                p_curr_dir->make_file(token2, file_size);
            }
        }
    }

    //p_root_dir->print();
    
    int total_size = p_root_dir->recurse_compute_total_size(max_dir_size);

    cout << total_size << "\n";

    return 0;
}
