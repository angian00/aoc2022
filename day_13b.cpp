#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
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


        friend bool operator==(const PacketData& data1, const PacketData& data2) {
            if (data1.type == PacketDataType::Int && data2.type == PacketDataType::Int) {
                return (data1.int_value == data2.int_value);
            
            } else if (data1.type == PacketDataType::List && data2.type == PacketDataType::List) {
                if (data1.list_values.size() != data2.list_values.size())
                    return false;

                for (int i_child=0; i_child < data1.list_values.size(); i_child++) {
                    if (data1.list_values[i_child] != data2.list_values[i_child])
                        return false;
                }

                return true;
            
            } else {
                return false;
            }
        }

        void print() {
            if (type == PacketDataType::Int)
                cout << int_value;
            else {
                cout << "[";
                for (int i=0; i < list_values.size(); i++) {
                    if (i > 0)
                        cout << ",";
                        
                    list_values[i].print();
                }
                cout << "]";
            }
        }

    private:
        PacketData() {}
        PacketData(int int_value): type(PacketDataType::Int), int_value(int_value) {}
        PacketData(vector<PacketData> list_values): type(PacketDataType::List), list_values(list_values) {}

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

bool cmp_data(PacketData data1, PacketData data2) {
    return PacketData::check_order(data1, data2) == DataOrderResult::OK;
}

int find_packet(vector<PacketData> packets, PacketData target) {
    for (int i_packet=0; i_packet < packets.size(); i_packet++) {
        if (packets[i_packet] == target)
            return i_packet;
    }

    assert(false);
    return -1;
}

int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);


    vector<PacketData> packets;

    while (getline(f, line))
    {
        PacketData p1 = PacketData::from_string(line);
        packets.push_back(p1);

        getline(f, line);
        PacketData p2 = PacketData::from_string(line);
        packets.push_back(p2);

        //empty separator line
        getline(f, line);
    }

    PacketData divider1 = PacketData::from_string("[[2]]");
    packets.push_back(divider1);
    PacketData divider2 = PacketData::from_string("[[6]]");
    packets.push_back(divider2);

    std::sort(packets.begin(), packets.end(), cmp_data);

    int div1_pos = find_packet(packets, divider1);
    int div2_pos = find_packet(packets, divider2);
    
    int decoder_key = (div1_pos+1) * (div2_pos+1);

    cout << decoder_key << "\n";

    return 0;
}
