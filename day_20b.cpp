#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>


using namespace std;


class EncryptedList {
    public:
        void add_value(int value) {
            data.push_back(value);
        }
        
        void print()
        {
            for (auto d: data)
                cout << d << ", ";
            
            cout << endl;
        }

        bool check_all_different() {
            for (int i=0; i < data.size(); i++) {
                int value = data[i];
        
                for (int j=0; j < i; j++) {
                    if (data[j] == value) {
                        cout << "repetition at: " << j << ", " << i << endl;
                        return false;
                    }
                }
            }

            return true;
        }

        void mix(int n_mixes, long long decryption_key) {
            vector<long long> new_data;
            
            for (int i=0; i < data.size(); i++) {
                data[i] *= decryption_key;
                index_map_old2new.push_back(i);
                index_map_new2old.push_back(i);
                new_data.push_back(0);
            }
            

            for (int i_mix=0; i_mix < n_mixes; i_mix++) {
                //bool do_trace = (i_mix == 2);
                bool do_trace = false;

                for (int i=0; i < data.size(); i++) {
                    long long value = data[i];
                    int curr_pos = index_map_old2new[i];

                    long long dest_pos = curr_pos + value;
                    if ( dest_pos < 0 || (dest_pos == 0 && curr_pos != 0) ) {
                        if (do_trace)
                            cout << "dest_pos too small: " << dest_pos << endl;
                        //dest_pos = dest_pos % (data.size() - 1);
                        dest_pos = (data.size() - 1) - abs(dest_pos) % (data.size() - 1);
                    }

                    
                    if (dest_pos >= (data.size()-1)) {
                        if (do_trace)
                            cout << "dest_pos too big: " << dest_pos << endl;
                        dest_pos = dest_pos % (data.size() - 1);
                        
                        if (dest_pos == 0)
                            dest_pos = (data.size() - 1);
                    }
                        
                    //cout << "after step " << i << "; value=" << value << " curr_pos=" << curr_pos << " dest_pos=" << dest_pos << endl;

                    if (dest_pos > curr_pos) {
                        for (int j=curr_pos+1; j <= dest_pos; j++) {
                            move_mapping(j, j-1);
                        }

                    } else {
                        for (int j=curr_pos-1; j >= dest_pos; j--) {
                            move_mapping(j, j+1);                        
                        }
                    }
                    
                    index_map_old2new[i] = dest_pos;
                    index_map_new2old[dest_pos] = i;
                    
                    if (is_mapping_corrupted())
                        break;


                    //DEBUG
                    if (do_trace) {
                        vector<int> tmp_data(data.size());
                        for (int j=0; j < data.size(); j++) {
                            int new_index = index_map_old2new[j];
                            tmp_data[new_index] = data[j];
                        }

                        cout << "new data: ";
                        for (int j=0; j < data.size(); j++)
                            cout << tmp_data[j] << ", ";
                        cout << endl;

                        cout << "index_map_old2new: ";
                        for (int j=0; j < data.size(); j++)
                            cout << index_map_old2new[j] << ", ";
                        cout << endl;

                        cout << "index_map_new2old: ";
                        for (int j=0; j < data.size(); j++)
                            cout << index_map_new2old[j] << ", ";
                        cout << endl;

                        cout << endl;
                    }
                    //
                }

                for (int i=0; i < data.size(); i++) {
                    int new_index = index_map_old2new[i];
                    new_data[new_index] = data[i];
                }

                // cout << "after " << i_mix+1 << " rounds of mixing: " << endl;
                // for (int i=0; i < data.size(); i++)
                //     cout << new_data[i] << ", ";
                // cout << endl;

            }
         

            data = new_data;
        }

        long long compute_total() {
            int zero_pos = find_value(0);
            
            long long total = 0;
            long long val1 = data[(zero_pos + 1000) % data.size()];
            total += val1;
            long long val2 = data[(zero_pos + 2000) % data.size()];
            total += val2;
            long long val3 = data[(zero_pos + 3000) % data.size()];
            total += val3;

            cout << "val1: " << val1 << endl;
            cout << "val2: " << val2 << endl;
            cout << "val3: " << val3 << endl;

            return total;
        }

    private:
        int find_value(int value) {
            for (int i=0; i < data.size(); i++) {
                if (data[i] == value)
                    return i;
            }

            return -1;
        }

        void move_mapping(int from, int to) {
            //cout << "move_mapping (before mod): from " << from << " to " << to << endl;

            int index_orig = index_map_new2old[from];
            //cout << "move_mapping: from " << from << " to " << to << " index_orig " << index_orig << " value " << data[index_orig] << endl;
            index_map_old2new[index_orig] = to;
            index_map_new2old[to] = index_orig;
        }

        bool is_mapping_corrupted() {
            for (int i=0; i < data.size(); i++) {
                if (index_map_old2new[i] < 0) {
                    cout << "old2new corrupted at index: " << i << endl;
                    return true;
                }

                if (index_map_new2old[i] < 0) {
                    cout << "new2old corrupted at index: " << i << endl;
                    return true;
                }
            }

            return false;
        }

        vector<long long> data;
        vector<int> index_map_old2new;
        vector<int> index_map_new2old;

};


int main(int argc, char *argv[])
{
    string filename = argv[1];
    
    ifstream f(filename);
    string line;

    if (!f)
        throw std::system_error(errno, std::system_category(), "failed to open "+ filename);


    EncryptedList enc_list;

    while (getline(f, line))
    {
        enc_list.add_value(stoi(line));
    }

    //cout << "all values are different? " << enc_list.check_all_different() << endl;

    //cout << "Starting values: " << endl;
    //enc_list.print();
    cout << endl;

    clock_t t_start = clock();

    int n_mixes = 10;
    long long decryption_key = 811589153LL;
    //int n_mixes = 1;
    //long long decryption_key = 1;
    enc_list.mix(n_mixes, decryption_key);
    auto total_coords = enc_list.compute_total();
    clock_t t_end = clock();
    
    //cout << "Final values: " << endl;
    //enc_list.print();
    cout << "total_coords: " << total_coords << endl;
    cout << "Execution time: " << (double) (t_end - t_start) / CLOCKS_PER_SEC << " seconds" << endl;

    return 0;
}
