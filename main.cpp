/*
 * Code by Alexander Roustai for mp2
 *
 */
#include <stdlib.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <cmath>
#include <vector>

using namespace std;
string::size_type sz;

//Initialized variables that will eb called through-out the file
int counter = 0;
char line[128];
char branch[128];
char taken[1];
int n_bit;
int bit_map = 0;
int prediction = 0;
int m1;
int m2;
int k;
int n;
int misprediction = 0;
int table_bounds;

//g_address, concatanated into a string and then read out
string g_add;

//Branch and Operation History
vector<string> taken_history;
vector<string> branch_history;

//Hybrid Table made me re-design significant parts of code
vector<int> g_table;
vector<int> b_table;
vector<int> h_table;
vector<int> global_history;
vector<int> b_history;

//Used for reading out instructions
typedef struct {
    char *branch;
    char *taken;
}line_feed;

void smith_pred(){
    //Initialized the inputs before running
    double n_bit_size = pow(2, double(n_bit));
    int pred_flag = 0; //branch bit_map
    bit_map = pow(2,n_bit-1);

    //Run and decipher based on the read in
    for(unsigned long i=0; i < taken_history.size(); i++) {
        char taken = taken_history[i][0];
        //printf("%c", taken);
        if (bit_map >= (n_bit_size / 2)) {
            pred_flag = 1;
        } else
            pred_flag = 0;

        //Taken, Not Taken logic
        if (taken == 't') {
            if (pred_flag == 1) {
                prediction++;
                bit_map++;
                if (bit_map == n_bit_size) {
                    bit_map = n_bit_size - 1;
                }
            }
            if (pred_flag == 0) {
                misprediction++;
                bit_map++;
                if (bit_map < 0) {
                    bit_map = 0;
                }
            }
        }

        if (taken == 'n') {
            if (pred_flag == 0) {
                prediction++;
                bit_map--;
                if (bit_map < 0) {
                    bit_map = 0;
                }
            }
            if (pred_flag == 1) {
                misprediction++;
                bit_map--;
                if (bit_map >= n_bit_size) {
                    bit_map = n_bit_size - 1;
                }
            }

        }
    }
}

void g_share(){
    //get size of table
    int table_size = pow(2,m1);
    for(int i = 0; i < table_size; i++){
        g_table.push_back(4);
    }

    //Used for global history
    string temp;
    for(int i =0; i < n; i++){
        temp.push_back('0');
        global_history.push_back(0);
    }


    for(unsigned long i=0; i < taken_history.size(); i++) {
        //decode current branch history
        long address = stol(branch_history[i], &sz, 16);
        address = address / 4;
        long n_bits_end     = address % long(pow(2,n));
        long n_bits_begin   = address / long(pow(2,n));
        long n_dec          = stol(temp, nullptr,2);
        long xor_bit        = n_bits_end ^ n_dec;
        long address_conv = n_bits_begin * (pow(2,n)) + xor_bit;
        long index        = address_conv % long(pow(2,m1));


        char taken = taken_history[i][0];
        int pred_flag = 0;
        if (g_table[index] >= 4) {
            pred_flag = 1;
        } else
            pred_flag = 0;

        //Taken/ Not Taken logic
        if (taken == 't') {
            if (pred_flag == 1) {
                prediction++;
                g_table[index] = g_table[index]+1;
                //printf("g_table index %d\n", g_table[index]);

                if (g_table[index]  >7) {
                    g_table[index] = 7;
                }
            }
            if (pred_flag == 0) {
                misprediction++;
                g_table[index] = g_table[index]+1;
                if (g_table[index] > 7) {
                    g_table[index] = 7;
                }
            }
            global_history.insert(global_history.begin(), 1);
        }

        if (taken == 'n') {
            if (pred_flag == 0) {
                prediction++;
                g_table[index] = g_table[index]-1;
                if (g_table[index] < 0) {
                    g_table[index] = 0;
                }
            }
            if (pred_flag == 1) {
                misprediction++;
                g_table[index] = g_table[index]-1;
                if (g_table[index] < 0) {
                    g_table[index] = 0;
                }
            }
        global_history.insert(global_history.begin(), 0);
        }

        global_history.pop_back();
        temp.clear();
        for(int i =0; i<global_history.size();i++){
            if(global_history[i] ==0){
                temp.push_back('0');
            }
            else
                temp.push_back('1');
        }

        //printf("index is: %s\n", temp.c_str());

    }
}

void bimodal(){
    //Initialize Table (we re-use Gshare table here for efficiency)
    int table_size = pow(2,m2);
    for(int i = 0; i < table_size; i++){
        g_table.push_back(4);
    }

    for(unsigned long i=0; i < taken_history.size(); i++) {
        //decode current branch history
        char taken = taken_history[i][0];

        long address = stol(branch_history[i], &sz, 16);
        address           = address / 4;
        long index        = address % long(pow(2,m2));

        //Prediction flag is used to determine taken/not-taken history
        int pred_flag = 0;
        if (g_table[index] >= 4) {
            pred_flag = 1;
        } else
            pred_flag = 0;

        if (g_table[index] >= 4) {
            pred_flag = 1;
        } else
            pred_flag = 0;

        //printf("%d\n", bit_map);
        if (taken == 't') {
            if (pred_flag == 1) {
                prediction++;
                g_table[index] = g_table[index]+1;

                if (g_table[index]  >7) {
                    g_table[index] = 7;
                }
            }
            if (pred_flag == 0) {
                misprediction++;
                g_table[index] = g_table[index]+1;
                if (g_table[index] > 7) {
                    g_table[index] = 7;
                }
            }
        }

        if (taken == 'n') {
            if (pred_flag == 0) {
                prediction++;
                g_table[index] = g_table[index]-1;
                if (g_table[index] < 0) {
                    g_table[index] = 0;
                }
            }
            if (pred_flag == 1) {
                misprediction++;
                g_table[index] = g_table[index]-1;
                if (g_table[index] < 0) {
                    g_table[index] = 0;
                }
            }
        }

    }
}

void h_initialize(){
    //Initialize Bimodal and Gshare for Hybrid
    int table_size_g = pow(2,m1);
    for(int i = 0; i < table_size_g; i++){
        g_table.push_back(4);
    }

    for(int i =0; i < n; i++){
        g_add.push_back('0');
        global_history.push_back(0);
    }

    //b_initialize
    int table_size_b = pow(2,m2);
    for(int i = 0; i < table_size_b; i++){
        b_table.push_back(4);
    }
}

long g_address(int i){
    //Find G-Share address, may or may not be used
    long address = stol(branch_history[i], &sz, 16);
    address = address / 4;
    long n_bits_end     = address % long(pow(2,n));
    long n_bits_begin   = address / long(pow(2,n));
    long n_dec          = stol(g_add, nullptr,2);
    long xor_bit        = n_bits_end ^ n_dec;
    long address_conv = n_bits_begin * (pow(2,n)) + xor_bit;
    long index        = address_conv % long(pow(2,m1));

    return index;
}

long b_address(int i){
    char taken = taken_history[i][0];
    //Find bimodal address, may or may not be used
    long address = stol(branch_history[i], &sz, 16);
    address           = address / 4;
    long index        = address % long(pow(2,m2));

    return index;
}

bool g_eval(int i, long index){
    bool guess;
    char taken = taken_history[i][0];
    int pred_flag = 0;
    if (g_table[index] >= 4) {
        pred_flag = 1;
    } else
        pred_flag = 0;

    //
    if (taken == 't') {
        if (pred_flag == 1) {
            guess = true;
            //g_table[index] = g_table[index]+1;
            //printf("g_table index %d\n", g_table[index]);

            if (g_table[index]  >7) {
                g_table[index] = 7;
            }
        }
        if (pred_flag == 0) {
            guess = false;
            //g_table[index] = g_table[index]+1;
            if (g_table[index] > 7) {
                g_table[index] = 7;
            }
        }
        global_history.insert(global_history.begin(), 1);
    }

    if (taken == 'n') {
        if (pred_flag == 0) {
            guess = true;
            //g_table[index] = g_table[index]-1;
            if (g_table[index] < 0) {
                g_table[index] = 0;
            }
        }
        if (pred_flag == 1) {
            guess = false;
            //g_table[index] = g_table[index]-1;
            if (g_table[index] < 0) {
                g_table[index] = 0;
            }
        }
        global_history.insert(global_history.begin(), 0);
    }

    global_history.pop_back();
    g_add.clear();
    for(int i =0; i<global_history.size();i++){
        if(global_history[i] ==0){
            g_add.push_back('0');
        }
        else
            g_add.push_back('1');
    }

    return guess;
}

bool b_eval(int i, long index){

    bool guess;
    char taken = taken_history[i][0];
    int pred_flag = 0;
    if (b_table[index] >= 4) {
        pred_flag = 1;
    } else
        pred_flag = 0;

    if (b_table[index] >= 4) {
        pred_flag = 1;
    } else
        pred_flag = 0;

    //printf("%d\n", bit_map);
    if (taken == 't') {
        if (pred_flag == 1) {
            guess = true;
        }
        if (pred_flag == 0) {
            guess = false;
        }
    }

    if (taken == 'n') {
        if (pred_flag == 0) {
            guess = true;
        }
        if (pred_flag == 1) {
            guess = false;
        }
    }
    return guess;
}

void g_update_table( int i, long index) {
    int pred_flag = 0;
    if (g_table[index] >= 4) {
        pred_flag = 1;
    } else
        pred_flag = 0;

    //printf("g_table index %d\n", g_table[index]);

    char taken = taken_history[i][0];
    if (taken == 't') {
        if (pred_flag == 1) {
            prediction++;
            g_table[index] = g_table[index] + 1;
            if (g_table[index] > 7) {
                g_table[index] = 7;
            }
        }
        if (pred_flag == 0) {
            //misprediction++;
            g_table[index] = g_table[index] + 1;
            if (g_table[index] > 7) {
                g_table[index] = 7;
            }
        }
    }

    if (taken == 'n') {
        if (pred_flag == 0) {
            prediction++;
            g_table[index] = g_table[index] - 1;
            if (g_table[index] < 0) {
                g_table[index] = 0;
            }
        }
        if (pred_flag == 1) {
            //misprediction++;
            g_table[index] = g_table[index] - 1;
            if (g_table[index] < 0) {
                g_table[index] = 0;
            }
        }
    }
}

void b_update_table(int i, long index){

    int pred_flag = 0;
    if (g_table[index] >= 4) {
        pred_flag = 1;
    } else
        pred_flag = 0;

    char taken = taken_history[i][0];
    if (taken == 't') {
        if (pred_flag == 1) {
            b_table[index] = b_table[index] + 1;
            prediction++;
            //printf("g_table index %d\n", g_table[index]);

            if (b_table[index] > 7) {
                b_table[index] = 7;
            }
        }
        if (pred_flag == 0) {
            //misprediction++;
            b_table[index] = b_table[index] + 1;
            if (b_table[index] > 7) {
                b_table[index] = 7;
            }
        }
    }

    if (taken == 'n') {
        if (pred_flag == 0) {
            prediction++;
            b_table[index] = b_table[index] - 1;
            if (b_table[index] < 0) {
                b_table[index] = 0;
            }
        }
        if (pred_flag == 1) {
            //misprediction++;
            b_table[index] = b_table[index] - 1;
            if (b_table[index] < 0) {
                b_table[index] = 0;
            }
        }
    }
}

void hybrid() {
    h_initialize();
    bool g_pred, b_pred;


    //Initialize H-Table
    int table_size_h = pow(2, k);
    for (int i = 0; i < table_size_h; i++) {
        h_table.push_back(1);
    }

    for (unsigned long i = 0; i < taken_history.size(); i++) {
        //get three address (gshare, bimodal, hybrid)
        long g_index = g_address(i);
        long b_index = b_address(i);

        long address = stol(branch_history[i], &sz, 16);
        address = address / 4;
        long h_index = address % long(pow(2, k));

        //Make prediction (doesn't matter if we use it or not)
        g_pred = g_eval(i, g_index);
        b_pred = b_eval(i, b_index);

        //Gshare table is used
        if (h_table[h_index] >= 2) {
            g_update_table(i, g_index);
            if(g_pred == false){
                misprediction++;
            }

        }

        //Bimodal table used
        if (h_table[h_index] < 2) {
            b_update_table(i, b_index);
            if(b_pred == false){
                misprediction++;
            }
        }

        //the h_counter logic
        if (g_pred == true) {
            if (b_pred == false) {
                //printf("hi\n");
                h_table[h_index] = h_table[h_index] + 1;
                //printf("hin: %d\n", h_table[h_index]);
                if (h_table[h_index] > 3) {
                    h_table[h_index] = 3;
                }
            }
        }
        if (b_pred == true) {
            if (g_pred == false) {
                h_table[h_index] = h_table[h_index] - 1;
                //printf("hdec: %d\n", h_table[h_index]);
                if (h_table[h_index] < 0) {
                    h_table[h_index] = 0;
                }
            }
        }
    }
}
void read_file(const char *file_in){
    FILE *ifp;
    ifp = fopen(file_in, "r");
    line_feed *lineFeed;

    //Read file and save it to a vector
    while(fgets(line,sizeof(line), ifp) != NULL) {
        int num_matches = sscanf(line, "%s %s", branch, taken);
        counter++;
        if (num_matches != 2) {
            //printf("Malformed line \n");
        }

        else
            taken_history.push_back(taken);
            branch_history.push_back(branch);
    }
}

int main(int argc, char* argv[]) {

    string operation = argv[1];

    if(operation == "smith"){
        string b_in  = argv[2];
        const char *file_in = argv[3];
        n_bit = stoi(b_in, &sz);
        read_file(file_in);
        smith_pred();

        cout<<"COMMAND"<<endl;
        cout<<"./sim smith "<<b_in <<" "<< argv[3]<<endl;
        cout<<"OUTPUT"<<endl;
        cout<<"number of predictions:\t" <<counter <<endl;
        cout<<"number of mispredictions:\t"<< misprediction<<endl;
        float miss_rate = float(misprediction)/float(counter) * 10000;
        cout<<"misprediction rate:\t"<< fixed<<setprecision(2)<< round(miss_rate)/100<< "%"<<endl;
        cout<<"FINAL COUNTER CONTENT:\t"<< bit_map<<endl;
    }

    if(operation == "gshare") {
        string table_bounds_in = argv[2];
        m1 = stoi(table_bounds_in, &sz);
        string b_in  = argv[3];
        n = stoi(b_in, &sz);
        const char *file_in = argv[4];
        read_file(file_in);
        g_share();

        cout<<"COMMAND"<<endl;
        cout<<"./sim gshare "<<m1 <<" " << n << " "<< argv[4]<<endl;
        cout<<"OUTPUT"<<endl;
        cout<<"number of predictions:\t" <<counter <<endl;
        cout<<"number of mispredictions:\t"<< misprediction<<endl;
        float miss_rate = float(misprediction)/float(counter) * 10000;
        cout<<"misprediction rate:\t"<< fixed<<setprecision(2)<< round(miss_rate)/100<< "%"<<endl;
        cout<<"FINAL GSHARE CONTENTS"<<endl;
        for(int i = 0; i <g_table.size(); i++){
            cout<<i<<'\t'<<g_table[i]<<endl;
        }
    }

    if(operation == "bimodal") {
        string table_bounds_in = argv[2];
        m2 = stoi(table_bounds_in, &sz);
        const char *file_in = argv[3];
        read_file(file_in);
        bimodal();

        cout<<"COMMAND"<<endl;
        cout<<"./sim bimodal "<<m2 <<" "<< argv[3]<<endl;
        cout<<"OUTPUT"<<endl;
        cout<<"number of predictions:\t" <<counter <<endl;
        cout<<"number of mispredictions:\t"<< misprediction<<endl;
        float miss_rate = float(misprediction)/float(counter) * 10000;
        cout<<"misprediction rate:\t"<< fixed<<setprecision(2)<< round(miss_rate)/100<< "%"<<endl;
        cout<<"FINAL BIMODAL CONTENTS"<<endl;
        for(int i = 0; i <g_table.size(); i++){
            cout<<i<<'\t'<<g_table[i]<<endl;
        }
    }

    if(operation == "hybrid") {
        string k_in = argv[2];
        k = stoi(k_in, &sz);
        string table_bounds_in = argv[3];
        m1 = stoi(table_bounds_in, &sz);
        string b_in  = argv[4];
        n = stoi(b_in, &sz);
        string m2_in = argv[5];
        m2 = stoi(m2_in, &sz);
        const char *file_in = argv[6];
        read_file(file_in);
        hybrid();

        cout<<"COMMAND"<<endl;
        cout<<"./sim hybrid "<<k<<" "<<m1 << " "<< n<< " "<<m2 << " "<< argv[6]<<endl;
        cout<<"OUTPUT"<<endl;
        cout<<"number of predictions:\t" <<counter <<endl;
        cout<<"number of mispredictions:\t"<< misprediction<<endl;
        float miss_rate = float(misprediction)/float(counter) * 10000;
        cout<<"misprediction rate:\t"<< round(miss_rate)/100<< "%"<<endl;

        cout<<"FINAL CHOOSER CONTENTS"<<endl;
        for(int i = 0; i <h_table.size(); i++){
            cout<<i<<'\t'<<h_table[i]<<endl;
        }
        cout<<"FINAL GSHARE CONTENTS"<<endl;
        for(int i = 0; i <g_table.size(); i++){
            cout<<i<<'\t'<<g_table[i]<<endl;
         }
        cout<<"FINAL BIMODAL CONTENTS"<<endl;
        for(int i = 0; i <b_table.size(); i++){
            cout<<i<<'\t'<<b_table[i]<<endl;
        }
    }

    return 0;
}
