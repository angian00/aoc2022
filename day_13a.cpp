#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <assert.h>


using namespace std;



bool is_digit(char c) {
    return (c >= '0' && c <= '9');
}


enum PacketDataType {
    Int,
    List,
};

enum DataOrderResult {
    OK,
    KO,
    Undecided,
};


class PacketData {
    public:
        static PacketData from_string(string data_str)
        {
            int start_index = 0;
            return PacketData::parse(data_str, &start_index);
        }


        static DataOrderResult check_order (const PacketData& data1, const PacketData& data2)
        {
            if (data1.type == PacketDataType::Int && data2.type == PacketDataType::Int) {
                if (data1.int_value < data2.int_value)
                    return DataOrderResult::OK;
                if (data1.int_value > data2.int_value)
                    return DataOrderResult::KO;
                
                return DataOrderResult::Undecided;
            
            } else if (data1.type == PacketDataType::List && data2.type == PacketDataType::List) {
                int i_child;
                for (i_child=0; i_child < data1.list_values.size(); i_child++) {
                    if (i_child >= data2.list_values.size())
                        return DataOrderResult::KO;
                    
                    DataOrderResult child_order = check_order(data1.list_values[i_child], data2.list_values[i_child]);
                    if (child_order == DataOrderResult::OK)
                        return DataOrderResult::OK;
                    if (child_order == DataOrderResult::KO)
                        return DataOrderResult::KO;
                }

                if (i_child < data2.list_values.size())
                    return DataOrderResult::OK;

                return DataOrderResult::Undecided;
            
            } else if (data1.type == PacketDataType::Int) {
                return PacketData::check_order(PacketData::IntToList(data1), data2);

            } else {
                return PacketData::check_order(data1, PacketData::IntToList(data2));
            }

        }


        void print() {
            print_recurse(0);
        }

    private:
        PacketData() {}
        PacketData(int int_value): type(PacketDataType::Int), int_value(int_value) {}
        PacketData(vector<PacketData> list_values): type(PacketDataType::List), list_values(list_values) {}

        void print_recurse(int recurse_level) {
            for (int i=0; i < recurse_level; i++) {
                cout << "  ";
            }
        }

        static PacketData parse(string data_str, int *p_start_index) {
            int start_index = *p_start_index;
            //cout << "parse(" << start_index << ")" << endl;

            char c = data_str[start_index];
            int index = start_index;

            if (is_digit(c)) {
                while (index < data_str.length() && is_digit(data_str[index]))
                    index ++;

                string int_str = data_str.substr(start_index, index - start_index);
                
                *p_start_index = index;
                return PacketData(stoi(int_str));
            }

            
            assert(data_str[index] == '[');
            index ++;
            
            vector<PacketData> list_values;
            while (index < data_str.length()) {
                c = data_str[index];
                if (is_digit(c)) {
                    start_index = index;
                    while (index < data_str.length() && is_digit(data_str[index]))
                        index ++;

                    string int_str = data_str.substr(start_index, index - start_index);
                    list_values.push_back(PacketData(stoi(int_str)));
                }

                else if (c == ',') {
                    //go to next element
                    index ++;
                
                } else if (c == '[') {
                    PacketData child = PacketData::parse(data_str, &index);
                    list_values.push_back(child);

                } else {
                    // data_str[index] == ']'
                    break;
                }
            }

            assert(data_str[index] == ']');
            index ++;

            *p_start_index = index;
            return PacketData(list_values);
        }

        static PacketData IntToList(PacketData int_data) {
            assert(int_data.type == PacketDataType::Int);

            vector<PacketData> list_values;
            list_values.push_back(int_data);

            PacketData list_data = PacketData(list_values);

            return list_data;
        }

        PacketDataType type;
        vector<PacketData> list_values;
        int int_value;
};


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);

    int tot_ordered = 0;
    int i_pair = 1;
    while (getline(f, line))
    {
        PacketData p1 = PacketData::from_string(line);
        
        getline(f, line);
        PacketData p2 = PacketData::from_string(line);

        if (PacketData::check_order(p1, p2) != DataOrderResult::KO)
            tot_ordered += i_pair;

        //empty separator line
        getline(f, line);
        i_pair ++;
    }

    cout << tot_ordered << "\n";

    return 0;
}
