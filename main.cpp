// hlsyn.cpp : Defines the entry point for the application.
//

//#include "hlsyn.h"
//#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

//global variables
string ins_list = string();        //comma delimited list of inputs to the circuit
int circuit_clocks = 0;            //number of clock periods required to schedule this circuit
int l_cstrt = 0;                //latency constraint as specified by input
int op_num = 0;                    //number of operations or node required by the input
int sum_count_DPC = 0;        //count the datapath components, which is sum of count_DPC[]


struct dp_comp {                //Data Path Component (DPC) structure with attributes for DPC
    //populated by parsing routine
    int order;                    //order as received from text file
    int function;                //enumeration of the components ( ALU = 0, MUL = 1, Logic/Logical = 2, Div = 3)
    string dp_ins_str;            //comma delimited list of inputs
    string dp_ins[3];            //array of inputs
    string dp_outs_str;            //comma delimited list of outputs
    string dp_outs[3];            //array of outputs
    //populated by sequencing routine
    int seq_clock;                //non-scheduled sequence order number
    int pred[2];
    int succ[2];
    //poplulate by ASAP routine
    int ASAP_clock;                //clock cycle for ASAP schedule
    //populate by ALAP routine
    int ALAP_clock;                //clock cycle for ALAP schedule
    //populate by width routine
    int width;                    //time-frame width
    string out_line;            //output line to be sent to output file (verilog file)
    //populate by total force routine
    float t_force;                //output value of total force = self-force + predecessor force + successor force
    int FDSched;                //Force Direceted Schedule, time period in which operation is scheduled
    int low_f_period;            //period with lowest force for this operation
    //populate by self force routine
    float s_force;
    int op_loc;                    //index of this node's data in the the specific resource probability table
    int FDSch;                    //period in which node is schedule by FDS

    //delete later
    int i_clock;                //clock latency number
    int d_width;                //datpath width
    float latency;                //value from estimated latency table, used to find critical datapath
    int top_order;                //topological order used to find critical datapath

};
struct dp_comp op_list[40];    //create array of above structure


//global structure contains attributes of the Circuit
struct cir_desc {            //circuit description
    string inp_str;            //input string
    string ins[40];            //array of inputs to the circuit
    int inp_count;            //number of inputs to the circuit
    string inp_type;        //Int or UInt
    string out_str;
    string outs[40];            //array of outputs of the circuit
    string var_str;
    string wires[4];
    string reg[40];
};
struct cir_desc cir_list;

double MUL_op[4][20] = {0.0};     // Initialize 2D array to all 0
double ALU_op[4][20] = {0.0};     // Initialize 2D array to all 0
double Logic_op[4][20] = {0.0};   // Initialize 2D array to all 0
double DIV_op[4][20] = {0.0};     // Initialize 2D array to all 0

double MUL_dist[20] = {0.0};    //Initialize  array to all 0
double ALU_dist[20] = {0.0};    //Initialize  array to all 0
double Logic_dist[20] = {0.0};  //Initialize  array to all 0
double DIV_dist[20] = {0.0};    //Initialize  array to all 0

void get_in_out_data(string str, string &str1, string &str2, string &str3, string &str4, int &bitsize) {
    size_t foundstr1, foundstr2, foundstr3;

    foundstr1 = str.find("input");
    if (foundstr1 != string::npos) {
        if ((foundstr1 = str.find("UInt")) != string::npos) {
            str = str.substr(foundstr1 + 4);
            cir_list.inp_type = "UInt";
        } else if ((foundstr2 = str.find("Int")) != string::npos) {
            str = str.substr(foundstr2 + 3);
            cir_list.inp_type = "Int";
            str4 = "1";
        }
        foundstr3 = str.find(" ");
        bitsize = stoi(str.substr(0, foundstr3));
        if (foundstr3 != string::npos) {
            str1 = str.substr(foundstr3 + 1);
        }
    }
    foundstr1 = str.find("output");
    if (foundstr1 != string::npos) {
        if ((foundstr1 = str.find("UInt")) != string::npos) {
            str = str.substr(foundstr1 + 4);
            cir_list.inp_type = "UInt";
        } else if ((foundstr2 = str.find("Int")) != string::npos) {
            str = str.substr(foundstr2 + 3);
            cir_list.inp_type = "Int";
            str4 = "1";
        }
        foundstr3 = str.find(" ");
        bitsize = stoi(str.substr(0, foundstr3));
        if (foundstr3 != string::npos) {
            str2 = str.substr(foundstr3 + 1);
        }
    }
    foundstr1 = str.find("variable");
    if (foundstr1 != string::npos) {
        if ((foundstr1 = str.find("UInt")) != string::npos) {
            str = str.substr(foundstr1 + 4);
            cir_list.inp_type = "UInt";
        } else if ((foundstr2 = str.find("Int")) != string::npos) {
            str = str.substr(foundstr2 + 3);
            cir_list.inp_type = "Int";
            str4 = "1";
        }
        foundstr3 = str.find(" ");
        bitsize = stoi(str.substr(0, foundstr3));
        if (foundstr3 != string::npos) {
            str3 = str.substr(foundstr3 + 1);
        }
    }
    return;
}


void get_ops_data(string str, int &temp, int &function) {
    size_t found, found1, found2, found3, found4, found5, found6, found7;
    size_t found8, found9, found10, found11;
    int sum_count_DPC = 0;        //count the datapath components, which is sum of count_DPC[]

    found = str.find(" = "); // detemine which operation is being performed
    if ((found != string::npos) && (found < 50)) {
        str = str.substr(found + 2);
        found1 = str.find(" + ");                    //select ADD or INC
        if (found1 != string::npos) {
            //temp = 1;
            found1 = str.find("+ 1");
            if (found1 != string::npos) {
                temp = 1;
                function = 0;
                //count_DPC[10]++;							//count instances of INC
            } else {
                temp = 2;
                function = 0;
                //count_DPC[1]++;								//count instances of ADD
            }
        }
        found2 = str.find(" - ");                    //select SUB or DEC
        if (found2 != string::npos) {
            //temp = 2;
            found2 = str.find("- 1");
            if (found2 != string::npos) {
                temp = 3;
                function = 0;
                //count_DPC[11]++;							//count instances of DEC
            } else {
                temp = 4;
                function = 0;
                //count_DPC[2]++;							//count instances of SUB
            }
        }
        found3 = str.find(" * ");                    //select MUL
        if (found3 != string::npos) {
            temp = 5;
            function = 1;
            //count_DPC[3]++;							//count instances of MUL
        }
        found4 = str.find(" / ");                    //select DIV
        if (found4 != string::npos) {
            temp = 6;
            function = 3;
            //count_DPC[8]++;							//count instances of DIV
        }
        found5 = str.find(" % ");                    //select MOD
        if (found5 != string::npos) {
            temp = 7;
            function = 3;
            //count_DPC[9]++;							//count instances of MOD
        }
        found6 = str.find(" << ");                    //select shift left, SHL
        if (found6 != string::npos) {
            temp = 8;
            function = 2;
            //count_DPC[7]++;							//count instances of SHL
        }
        found7 = str.find(" >> ");                    //select shift right, SHR
        if (found7 != string::npos) {
            temp = 9;
            function = 2;
            //count_DPC[6]++;							//count instances of SHR
        }
        found8 = str.find(" ? ");                    //select MUX
        if (found8 != string::npos) {
            temp = 10;
            function = 2;
            //count_DPC[5]++;							//count instances of MUX
        }
        found9 = str.find(" == ");                    //select COMP, eq output
        if (found9 != string::npos) {
            temp = 11;
            function = 2;
            //count_DPC[4]++;							//count instances of COMP
        }
        found10 = str.find(" < ");                    //select COMP, lt output
        if ((found10 != string::npos) && (temp != 6)) {
            temp = 12;
            function = 2;
            //count_DPC[4]++;							//count instances of COMP
        }
        found11 = str.find(" > ");                    //select COMP, gt output
        if ((found11 != string::npos) && (temp != 7)) {
            temp = 13;
            function = 2;
            //count_DPC[4]++;							//count instances of COMP
        }
        if ((found != string::npos) && (temp == 0))        //select REG
        {
            temp = 15;
            function = 2;
            //count_DPC[0]++;							//count instances of REG
        }
        //count_dpc++;
    }
    return;
}

int opcheck(string str) //operation check
{
    int e = 0;
    size_t h;
    string op;
    string ops[11] = {"+", "-", "*", "/", "%", "<", ">", "==", "<<", ">>", "?"};
    h = str.find(" ");
    str = str.substr(h + 1);
    h = str.find(" ");
    str = str.substr(h + 1);
    h = str.find(" ");
    str = str.substr(h + 1);
    op = str.substr(0, (str.find(" ")));
    for (int b = 0; b < 12; b++) {
        if ((op != ops[b]) && (op != " ")) {
            e = 1;
        } else {
            e = 0;
            break;
        }
    }
    return (e);
}

void
varcheck(string str, string str1, string str2, string str3, string w, string x, string y, string z, int &ew, int &ex,
         int &ey, int &ez) //operation check
{
    int e = 0;
    //int dw = 0, dx = 0, dy = 0, dz = 0;
    size_t found1, found2, found3;
    if (ex != 0) {
        found1 = str1.find(x);
        found2 = str2.find(x);
        found3 = str3.find(x);
        if ((found1 != string::npos) || (found2 != string::npos) || (found3 != string::npos))
            ex = 0;
        else
            ex = 2;
    }
    if ((y != "") && (ey != 0)) {
        found1 = str1.find(y);
        found2 = str2.find(y);
        found3 = str3.find(y);
        if ((found1 != string::npos) || (found2 != string::npos) || (found3 != string::npos))
            ey = 0;
        else
            ey = 2;
    } else
        ey = 0;
    if (ez != 0) {
        found1 = str1.find(z);
        found2 = str2.find(z);
        found3 = str3.find(z);
        if ((found1 != string::npos) || (found2 != string::npos) || (found3 != string::npos))
            ez = 0;
        else
            ez = 2;
    }
    if ((w != "") && (ew != 0)) {
        found1 = str1.find(w);
        found2 = str2.find(w);
        found3 = str3.find(w);
        if ((found1 != string::npos) || (found2 != string::npos) || (found3 != string::npos))
            ew = 0;
        else
            ew = 2;
    } else
        ew = 0;
    return;
}
void iovalues(string str, string &x, string &y, string &z) //find input and output variables for structure
{
    z = str.substr(0, str.find(" "));
    str = str.substr((str.find(" ") + 3));
    x = str.substr(0, str.find(" "));
    str = str.substr((str.find(" ") + 3));
    y = str.substr(0, str.find(" "));
    return;
}

void iovaluesmux(string str, string &w, string &x, string &y, string &z) //find input and output variables for structure
{
    z = str.substr(0, str.find(" "));
    str = str.substr((str.find(" ") + 3));
    w = str.substr(0, str.find(" "));
    str = str.substr((str.find(" ") + 3));
    x = str.substr(0, str.find(" "));
    str = str.substr((str.find(" ") + 3));
    y = str.substr(0, str.find(" "));
    return;
}

void iovaluesreg(string str, string &x, string &z) //find input and output variables for structure
{
    z = str.substr(0, str.find(" "));
    str = str.substr((str.find(" ") + 3));
    x = str.substr(0, str.find(" "));
    return;
}

void iovaluesshift(string str, string &x, string &y, string &z) //find input and output variables for structure
{
    z = str.substr(0, str.find(" "));
    str = str.substr((str.find(" ") + 3));
    x = str.substr(0, str.find(" "));
    str = str.substr((str.find(" ") + 4));
    y = str.substr(0, str.find(" "));
    return;
}

void iovaluescomp(string str, string &x, string &y, string &z) //find input and output variables for structure
{
    int tempinc;
    size_t yes;
    z = str.substr(0, str.find(" "));
    str = str.substr((str.find(" ") + 3));
    x = str.substr(0, str.find(" "));
    yes = (str.find("=="));
    if (yes != string::npos) {
        tempinc = 4;
    } else {
        tempinc = 3;
    }
    str = str.substr((str.find(" ") + tempinc));
    y = str.substr(0, str.find(" "));
    return;
}

void iovaluesif(string str, string &x, string &outsize) //find input and output variables for if structure
{
    size_t found;

    found = str.find("(");
    if ((found != string::npos) && (found < str.length()))
        /*z = str.substr(0, str.find(" "));
        str = str.substr((str.find(" ") + 3));
        x = str.substr(0, str.find(" "));*/
        return;
}

void parse_cir_inputs(void) {
    string temp_str = string();
    int p = 0;
    int findLoc = 0;
    int beg_pos = 0;
    int end_pos = 0;
    int index = 0;
    string var_str = string();

    cir_list.inp_count = 0;
    temp_str = cir_list.inp_str;
    temp_str = temp_str + '\0';            //terminate with NULL

    end_pos = 0;
    p = -1;
    do {
        p++;
        while (((temp_str[end_pos] == ' ') || (temp_str[end_pos] == ',')) && (temp_str[end_pos])) {
            end_pos++;            //step over spaces and comma to eliminate from variable
        }
        beg_pos = end_pos++;
        while (temp_str[end_pos] != ',' && (temp_str[end_pos])) {
            ++end_pos;
        }
        cir_list.ins[p] = temp_str.substr(beg_pos, end_pos - beg_pos);
        cir_list.inp_count++;
    } while (temp_str[end_pos] && p < 40);

    temp_str = cir_list.out_str;
    temp_str = temp_str + '\0';            //terminate with NULL

    end_pos = 0;
    p = -1;
    do {
        p++;
        while (((temp_str[end_pos] == ' ') || (temp_str[end_pos] == ',')) && (temp_str[end_pos])) {
            end_pos++;            //step over spaces and comma to eliminate from variable
        }
        beg_pos = end_pos++;
        while (temp_str[end_pos] != ',' && (temp_str[end_pos])) {
            ++end_pos;
        }
        cir_list.outs[p] = temp_str.substr(beg_pos, end_pos - beg_pos);
    } while (temp_str[end_pos] && p < 40);
}

void asap_sequence(int &latency, int &error) {
    int b = 0, d = 0, h = 0, k = 0;
    int yes = 0, xyes = 0, yyes = 0, wyes = 0;
    int clock = 0, more = 0, cycle = 0, func = 1, temp = 1;
    string dpc_var1 = "";
    string dpc_var2 = "";
    string dpc_var3 = "";
    string str = "";

    while (clock <= latency) {
        while (op_list[d].dp_outs_str != "")        //compare data path components with the inputs to the circuit
        {
            if (op_list[d].ASAP_clock == 0) {
                for (b = 0; b < cir_list.inp_count; b++) {
                    str = cir_list.ins[b];
                    dpc_var1 = op_list[d].dp_ins[0];
                    if (dpc_var1 == str) {
                        xyes++;
                        break;
                    }
                }
                h = 0;
                while ((op_list[h].dp_outs_str != "") && (xyes == 0) && (clock != 0)) {
                    if ((h != d) && (op_list[h].ASAP_clock <= clock) && (op_list[h].ASAP_clock != 0) &&
                        (op_list[h].ASAP_clock >= 0)) {
                        if (dpc_var1 == op_list[h].dp_outs[0]) {
                            xyes++;
                            if ((op_list[h].function == 1) &&
                                ((clock - op_list[h].ASAP_clock) < 2))            //multiplier operation
                            {
                                more = 1;
                            }
                            if ((op_list[h].function == 3) &&
                                ((clock - op_list[h].ASAP_clock) < 3))            //division or modulus operation
                            {
                                more = 2;
                            }
                            break;
                        }
                    }
                    h++;
                }
                for (b = 0; b < cir_list.inp_count; b++) {
                    if (op_list[d].dp_ins[1] != "") {
                        str = cir_list.ins[b];
                        dpc_var2 = op_list[d].dp_ins[1];
                        if (dpc_var2 == str) {
                            yyes++;
                            break;
                        }
                    } else
                        break;
                }
                h = 0;
                while ((op_list[h].dp_outs_str != "") && (yyes == 0) && (clock != 0)) {
                    if ((h != d) && (op_list[h].ASAP_clock <= clock) && (op_list[h].ASAP_clock != 0) &&
                        (op_list[h].ASAP_clock >= 0)) {
                        if (dpc_var2 == op_list[h].dp_outs[k]) {
                            yyes++;
                            if ((op_list[h].function == 1) &&
                                ((clock - op_list[h].ASAP_clock) < 2))            //multiplier operation
                            {
                                more = 1;
                            }
                            if ((op_list[h].function == 3) &&
                                ((clock - op_list[h].ASAP_clock) < 3))            //division or modulus operation
                            {
                                more = 2;
                            }
                            break;
                        }
                    }
                    h++;
                }
                for (b = 0; b < cir_list.inp_count; b++) {
                    if (op_list[d].dp_ins[2] != "") {
                        str = cir_list.ins[b];
                        dpc_var3 = op_list[d].dp_ins[2];
                        if (dpc_var3 == str) {
                            wyes++;
                            break;
                        }
                    } else
                        break;
                }
                h = 0;
                while ((op_list[h].dp_outs_str != "") && (wyes == 0) && (clock != 0)) {
                    if ((h != d) && (op_list[h].ASAP_clock <= clock) && (op_list[h].ASAP_clock != 0) &&
                        (op_list[h].ASAP_clock >= 0)) {
                        if (dpc_var3 == op_list[h].dp_outs[0]) {
                            wyes++;
                            if ((op_list[h].function == 1) &&
                                ((clock - op_list[h].ASAP_clock) < 2))            //multiplier operation
                            {
                                more = 1;
                            }
                            if ((op_list[h].function == 3) &&
                                ((clock - op_list[h].ASAP_clock) < 3))            //division or modulus operation
                            {
                                more = 2;
                            }
                            break;
                        }
                    }
                    h++;
                }
                yes = xyes + yyes + wyes;
                if ((yes == 2) && (dpc_var2 != "") && (dpc_var3 == ""))
                    op_list[d].ASAP_clock = op_list[d].ASAP_clock + more + clock + 1;
                if ((yes == 3) && (dpc_var2 != "") && (dpc_var3 != ""))
                    op_list[d].ASAP_clock = op_list[d].ASAP_clock + more + clock + 1;
                if ((yes == 1) && (dpc_var2 == "") && (dpc_var3 == ""))
                    op_list[d].ASAP_clock = op_list[d].ASAP_clock + more + clock + 1;
            }
            d++;
            yes = 0;
            xyes = 0;
            yyes = 0;
            wyes = 0;
            more = 0;
            dpc_var1 = "";
            dpc_var2 = "";
            dpc_var3 = "";
        }
        d = 0;
        clock++;
    }

    while (op_list[k].ASAP_clock != 0) {
        if (op_list[k].ASAP_clock > cycle)
            cycle = op_list[k].ASAP_clock;
        else
            cycle = cycle;
        k++;
    }
    k = 0;
    while (op_list[k].ASAP_clock != 0) {
        if (op_list[k].ASAP_clock == cycle) {
            if (op_list[k].function == 1)
                temp = 2;
            if (op_list[k].function == 3)
                temp = 3;
            if ((op_list[k].function == 0) || (op_list[k].function == 2))
                temp = 1;
        }
        if (temp > func)
            func = temp;
        else
            func = func;
        k++;
    }
    if ((cycle + func) > latency)
        error = 4;
    else
        error = 0;
    return;
}

void alap_sequence(int &latency) {
    int b = 0, d = 0, h = 0, k = 0, s = 0;
    int cycle = 0, delay = 0, node = 0;
    string str = "", temp = "";
    int vertex[40] = {};
    int p = 0;

    delay = latency;
    while (cir_list.outs[b] != "") {
        temp = cir_list.outs[b];
        //for (d = 0; d < delay; d++)
        while (op_list[d].dp_outs[0] != "") {
            if (op_list[d].dp_outs[0] == temp)
            {
                if (op_list[d].function == 1)
                    op_list[d].ALAP_clock = delay - 1;
                if (op_list[d].function == 3)
                    op_list[d].ALAP_clock = delay - 2;
                if ((op_list[d].function == 0) || (op_list[d].function == 2))
                    op_list[d].ALAP_clock = delay;
                vertex[s] = d;
                s++;
            }
            d++;
        }
        d = 0;
        b++;
    }
    d = 0;

    for (k = 0; k < s; k++) {
        node = vertex[k];
        for (h = 0; h < 3; h++) {
            b = 0;
            str = op_list[node].dp_ins[h];
            while (op_list[b].dp_outs[0] != "" && (str != "")) {
                temp = op_list[b].dp_outs[0];
                if (temp == str) {
                    op_list[b].ALAP_clock =
                            op_list[node].ALAP_clock - (op_list[node].ASAP_clock - op_list[b].ASAP_clock);
                    d = 0;
                    p = 0;
                    while (d < s) {
                        if (vertex[d] == b)
                            p = 1;
                        d++;
                    }
                    if (p == 0) {
                        vertex[s] = b;
                        s++;
                    }
                }
                b++;
            }
        }
    }

    k = 0;
    while (op_list[k].dp_outs_str != "") {
        op_list[k].width = (op_list[k].ALAP_clock - op_list[k].ASAP_clock) + 1;
        k++;
    }

    return;
}


//calculate the probability for each operation in each clock period
void op_prob(void) {

    int mul_count = 0;        //counters for respective operations
    int alu_count = 0;
    int logic_count = 0;
    int div_count = 0;
    int p = 0;                //count operations
    int clk_count = 0;        //count clock periods

/*    //inialize array variable to zero
    for (clk_count = 0; clk_count < 20; clk_count++)
    {
        for (p = 0; p < 4; p++)
        {
            MUL_op[p][clk_count] = 0;
            ALU_op[p][clk_count] = 0;
            Logic_op[p][clk_count] = 0;
            DIV_op[p][clk_count] = 0;
        }
        MUL_dist[clk_count] = 0;
        ALU_dist[clk_count] = 0;
        Logic_dist[clk_count] = 0;
        DIV_dist[clk_count] = 0;
    }

    p = 0;*/
    do {
        switch (op_list[p].function)        // (ALU = 0, MUL = 1, Logic / Logical = 2, Div = 3)
        {
            case 0:
                for (clk_count = 0; clk_count < l_cstrt; clk_count++) {
                    if (clk_count >= op_list[p].ASAP_clock - 1 && clk_count <= op_list[p].ALAP_clock - 1) {
                        ALU_op[alu_count][clk_count] = 1.0 / op_list[p].width;
                        op_list[p].op_loc = alu_count;
                    }
                }
                alu_count++;
                break;

            case 1:
                for (clk_count = 0; clk_count < l_cstrt; clk_count++) {
                    if (clk_count >= op_list[p].ASAP_clock - 1 && clk_count <= op_list[p].ALAP_clock - 1) {
                        MUL_op[mul_count][clk_count] = 1.0 / op_list[p].width;
                        op_list[p].op_loc = mul_count;
                    }
                }
                mul_count++;                                            //count multiplier operations
                break;

            case 2:
                for (clk_count = 0; clk_count < l_cstrt; clk_count++) {
                    if (clk_count >= op_list[p].ASAP_clock - 1 && clk_count <= op_list[p].ALAP_clock - 1) {
                        Logic_op[logic_count][clk_count] = 1.0 / op_list[p].width;
                        op_list[p].op_loc = logic_count;
                    }
                }
                logic_count++;
                break;

            case 3:
                for (clk_count = 0; clk_count < l_cstrt; clk_count++) {
                    if (clk_count >= op_list[p].ASAP_clock - 1 && clk_count <= op_list[p].ALAP_clock - 1) {
                        DIV_op[div_count][clk_count] = 1.0 / op_list[p].width;
                        op_list[p].op_loc = div_count;
                    }
                }
                div_count++;
                break;
        } //switch
        ++p;
    } while (op_list[p].function < 99);

    //sum the probabilities for each clock period
    //MUL
    for (clk_count = 0; clk_count < l_cstrt; clk_count++) {
        MUL_dist[clk_count] = MUL_op[0][clk_count] + MUL_op[1][clk_count] + MUL_op[2][clk_count] + MUL_op[3][clk_count];
    }
    //ALU
    for (clk_count = 0; clk_count < l_cstrt; clk_count++) {
        ALU_dist[clk_count] = ALU_op[0][clk_count] + ALU_op[1][clk_count] + ALU_op[2][clk_count] + ALU_op[3][clk_count];
    }
    //Logic
    for (clk_count = 0; clk_count < l_cstrt; clk_count++) {
        Logic_dist[clk_count] =
                Logic_op[0][clk_count] + Logic_op[1][clk_count] + Logic_op[2][clk_count] + Logic_op[3][clk_count];
    }
    //DIV
    for (clk_count = 0; clk_count < l_cstrt; clk_count++) {
        DIV_dist[clk_count] = DIV_op[0][clk_count] + DIV_op[1][clk_count] + DIV_op[2][clk_count] + DIV_op[3][clk_count];
    }
}


//given the node number and the clock period
//this function calculates the self-force
double calcSelfForce(int node, int t_period) {
    int k = 0;                //index clock periods
    int fun_type = 0;        //type of function from
    double self_force = 0;    //self force calculated, return this value
    double temp_force = 0;    //temporary local variable

    fun_type = op_list[node].function;            // (ALU = 0, MUL = 1, Logic / Logical = 2, Div = 3)

    switch (fun_type) {
        case 0:                                    //ALU
            for (k = 0; k < l_cstrt; k++) {
                if (t_period == k + 1)            //time period count starts at 1
                    temp_force = (ALU_dist[k]) *
                                 (1 - ALU_op[op_list[node].op_loc][k]);        //time period under consideration
                else
                    temp_force = (ALU_dist[k]) *
                                 (0 - ALU_op[op_list[node].op_loc][k]);        //not time period under consideration
                self_force = self_force + temp_force;
            }
            break;
        case 1:                                    //MUL
            for (k = 0; k < l_cstrt; k++) {
                if (t_period == k + 1)
                    temp_force = (MUL_dist[k]) *
                                 (1 - MUL_op[op_list[node].op_loc][k]);        //time period under consideration
                else
                    temp_force = (MUL_dist[k]) *
                                 (0 - MUL_op[op_list[node].op_loc][k]);        //not time period under consideration
                self_force = self_force + temp_force;
            }
            break;
        case 2:                                    //Logic
            for (k = 0; k < l_cstrt; k++) {
                if (t_period == k + 1)
                    temp_force = (Logic_dist[k]) *
                                 (1 - Logic_op[op_list[node].op_loc][k]);        //time period under consideration
                else
                    temp_force = (Logic_dist[k]) *
                                 (0 - Logic_op[op_list[node].op_loc][k]);        //not time period under consideration
                self_force = self_force + temp_force;
            }
            break;
        case 3:                                    //DIV
            for (k = 0; k < l_cstrt; k++) {
                if (t_period == k + 1)
                    temp_force = (DIV_dist[k]) *
                                 (1 - DIV_op[op_list[node].op_loc][k]);        //time period under consideration
                else
                    temp_force = (DIV_dist[k]) *
                                 (0 - DIV_op[op_list[node].op_loc][k]);        //not time period under consideration
                self_force = self_force + temp_force;
            }
            break;
    }

    return self_force;
}

float calcTForce(int node, int t_period) {
    int p = 0;
    int ps_node1 = 0;            //predecessor or successor node
    int ps_node2 = 0;
    double self_force = 0;
    double pred_force1 = 0;
    double pred_force2 = 0;
    double succ_force1 = 0;
    double succ_force2 = 0;
    double total_force = 0;

    self_force = calcSelfForce(node, t_period);

    //find predecessor forces
    if (t_period != 1)                        //must find predecessor forces when not in time period 1
    {
        ps_node1 = op_list[node].pred[0];
        if ((t_period - 1) == op_list[ps_node1].ASAP_clock) {
            pred_force1 = calcSelfForce(ps_node1, t_period - 1);
        }
        ps_node2 = op_list[node].pred[1];
        if ((t_period - 1) == op_list[ps_node2].ASAP_clock) {
            pred_force2 = calcSelfForce(ps_node2, t_period - 1);
        }
    }

    //find successor forces
    if (t_period != l_cstrt)                        //must find successor forces when not in last time period
    {
        ps_node1 = op_list[node].succ[0];
        if ((t_period + 1) == op_list[ps_node1].ASAP_clock) {
            succ_force1 = calcSelfForce(ps_node1, t_period + 1);
        }
        ps_node2 = op_list[node].succ[1];
        if ((t_period + 1) == op_list[ps_node2].ASAP_clock) {
            succ_force2 = calcSelfForce(ps_node2, t_period + 1);
        }
    }

    total_force = self_force + pred_force1 + pred_force2 + succ_force1 + succ_force2;

    return total_force;
}

void forceDSched(void) {
    int sched_count = 0;                            //count operations scheduled
    int p = 0;
    int k = 0;
    int low_node = 0;                                //node with least force
    double low_force = 0;                            //least force
    int low_period = 0;                                //period with least force, used with low node
    double forces[20][10];                    //array of forces, number of operations x latency constraint

    //first step is to schedule implicitly scheduled operations; operations with a width of 1
    for (p = 0; p < op_num; p++) {
        if (op_list[p].width == 1) {
            op_list[p].FDSched = op_list[p].ASAP_clock;
            sched_count++;                            //count operations scheduled
        }
    }

    while (sched_count < op_num)                    //until all operations are scheduled
    {
        //initialize all forces to bogus high value
        for (p = 0; p < op_num; p++)                //every node
        {
            for (k = 0; k < l_cstrt; k++)            //every clock period
            {
                forces[p][k] = 999.9;                //p is node, k is time period
            }
        }

        //find self force for each operation within its time frame
        for (p = 0; p < op_num; p++)        //every node
        {
            if (!op_list[p].FDSched)            //not scheduled yet
            {
                for (k = op_list[p].ASAP_clock - 1; k < op_list[p].ALAP_clock; k++) {
                    //forces[p][k] = calcSelfForce(p, k);				//p is node, k is time period
                    forces[p][k] = calcTForce(p, k);                //p is node, k is time period
                }
            }
        }

        low_force = 99.9;                    //start with bogus high value
        //find the operation with the least force
        for (p = 0; p < op_num; p++)        //every node
        {
            for (k = 0; k < l_cstrt; k++)    //every clock period
            {
                if (!op_list[p].FDSched)        //not scheduled yet
                {
                    if (forces[p][k] < low_force) {
                        low_force = forces[p][k];        //update low force
                        low_node = p;                    //update low node
                        low_period = k;                    //update low period
                    }
                }
            }
        }

        op_list[low_node].FDSched = low_period +
                                    1;                            //schedule this node, FDSched, ASAP_clock, ALAP clock start at 1
        op_list[low_node].low_f_period = low_period + 1;                    //record the low force period
        op_list[low_node].ASAP_clock = op_list[low_node].FDSched;            //update the time frame
        op_list[low_node].ALAP_clock = op_list[low_node].FDSched;
        sched_count++;                                        //count operations scheduled
    } //while (sched_count < op_num)						//until all operations are scheduled
}

int main(int argc, char *argv[]) {
    string filename, filename1, filename2, iline, oline, newline, str, str1, strc, strv;
    int latency = 0, function = 99;
    string str2, str3, str4, str5, stri, stro;
    string instr[20] = {}, outstr[20] = {}, wirestr[20] = {}, regstr[20] = {}, varstr[20] = {};
    string insize, outsize = "", w, x, y, z, w_dw[14] = {}, x_dw[14] = {}, y_dw[14] = {}, z_dw[14] = {};
    string node[40] = {};
    size_t found, foundif;
    size_t foundname1;        // , foundname2;
    int bittemp;
    int temp = 0, bitsize = 0, start = 0, i = 0, m = 0, DW[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    //array to count instances of each datapath component
    int s = 0, error = 0, here = 0, count_DPC[15] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    //int nw = 0, nx = 0, ny = 0, nz = 0, sw = 0, sx = 0, sy = 0, sz = 0;
    int ew, ex, ey, ez;
    int u = 0, sign_var[20] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int sum_count_DPC = 0;        //count the datapath components, which is sum of count_DPC[]
    int c_period_req = 0;        //number of clock periods required by schedule
    float cr_dp = 0.0;            //critical data path

    //initailize the op_list array
    for (i = 0; i < 40; i++) {
        op_list[i].function = 99;        //component enumeration uses 0 thru 12
        op_list[i].order = 0;
        op_list[i].top_order = 0;
        op_list[i].out_line = string();
        op_list[i].d_width = 0;
        op_list[i].ASAP_clock = 0;
        op_list[i].ALAP_clock = 0;
        op_list[i].latency = 0.0;
        op_list[i].i_clock = 0;
        op_list[i].dp_ins[0] = string();
        op_list[i].dp_ins[1] = string();
        op_list[i].dp_ins_str = string();
        op_list[i].dp_outs[0] = string();
        op_list[i].dp_outs[1] = string();
        op_list[i].dp_outs[2] = string();
        op_list[i].dp_outs_str = string();
        op_list[i].FDSched = 0;                //time period in which operation is scheduled, zero for NOT scheduled, first time period is 1
        op_list[i].low_f_period = 0;
        op_list[i].pred[0] = 0;
        op_list[i].pred[1] = 0;
        op_list[i].succ[0] = 0;
        op_list[i].succ[1] = 0;
    }
    i = 0;

    if (argc == 4 && argv[2] != 0)
    {
        filename1 = string(argv[1]);
        filename2 = string(argv[3]);
        latency = stoi(argv[2]);
    }
    else
    {
        cerr << "Program Terminated: Invalid number of arguments or Invalid latency";
        return 1;
    }

    /*cout << "Please enter filename: "; // generate output file
    cin >> filename1;*/
    ifstream myfile1(filename1); // open input file
/*
    cout << "Enter latency value (in cycles): "; //latency value input
    cin >> latency;
    l_cstrt = latency;
    if (latency == 0) {
        cout << "Missing latency Value ";
        cout << "Enter latency value (in cycles): "; //latency value input
        cin >> latency;
    }*/


    if (myfile1.is_open()) // open input file check and write to output file check
    {
        s = 0;
        int count_dpc = 0;                    //count the datapath components found
        while (getline(myfile1, iline)) //parse the input variables
        {
            while (iline.find(" ") == 0)
                iline = iline.substr((iline.find(" ")) + 1);    //remove extra preceding spaces
            while (iline.find("\t") == 0)
                iline = iline.substr((iline.find("\t")) + 1);    //remove preceding tab characters
            str1 = "";
            str2 = "";
            str3 = "";
            str4 = "0";
            w = "";
            x = "";
            y = "";
            z = "";
            temp = 0;
            found = iline.find("if");
            if ((found != string::npos) && (found < iline.length())) {
                foundif = iline.find("{");
                if ((foundif != string::npos) && (foundif < iline.length()))
                    newline = iline.substr(0, foundif - 1);
                node[s] = newline;    //add "if" statement to vertex/node array
                iovaluesif(newline, x, outsize);
                op_list[sum_count_DPC].order = sum_count_DPC + 1;
                op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
                op_list[sum_count_DPC].dp_outs_str = newline;
                op_list[sum_count_DPC].function = 2;
                sum_count_DPC++;
                s++;
            }
            found = iline.find("else");
            if ((found != string::npos) && (found < iline.length())) {
                node[s] = iline;    //add "else" statement to vertex/node array
                op_list[sum_count_DPC].order = sum_count_DPC + 1;
                op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
                op_list[sum_count_DPC].dp_outs_str = iline;
                op_list[sum_count_DPC].function = 2;
                sum_count_DPC++;
                s++;
            }
            if (iline != "") {
                foundname1 = iline.find("=");
                if (foundname1 != string::npos) {
                    node[s] = iline;    //add to vertex/node array
                    op_list[sum_count_DPC].order = sum_count_DPC + 1;
                    op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
                    op_list[sum_count_DPC].dp_outs_str = iline;

                    s++;
                    found = iline.find("for");
                    if (found == string::npos) {
                        get_ops_data(iline, temp, function);
                        op_list[sum_count_DPC].function = function;
                        count_DPC[temp]++;
                        error = opcheck(iline);
                        if (error == 1) {
                            cout << endl
                                 << " Invalid Operation" << endl;
                            system("pause");
                            return 2;
                        }
                        ew = 1;
                        ex = 1;
                        ey = 1;
                        ez = 1;
                        for (m = 0; m < i; m++) {
                            stri = instr[m];
                            stro = outstr[m];
                            strv = varstr[m];
                            switch (temp) {
                                case 10:
                                    iovaluesmux(iline, w, x, y, z);
                                    break;
                                case 15:
                                    iovaluesreg(iline, x, z);
                                    break;
                                case 1:
                                    iovaluesreg(iline, x, z);
                                    break;
                                case 3:
                                    iovaluesreg(iline, x, z);
                                    break;
                                case 11:
                                    iovaluescomp(iline, x, y, z);
                                    break;
                                case 12:
                                    iovaluescomp(iline, x, y, z);
                                    break;
                                case 13:
                                    iovaluescomp(iline, x, y, z);
                                    break;
                                case 8:
                                    iovaluesshift(iline, x, y, z);
                                    break;
                                case 9:
                                    iovaluesshift(iline, x, y, z);
                                    break;
                                default:
                                    iovalues(iline, x, y, z);
                            }
                            varcheck(iline, stri, stro, strv, w, x, y, z, ew, ex, ey, ez);
                            op_list[count_dpc].dp_ins[0] = x;
                            if (y != "")
                                op_list[count_dpc].dp_ins[1] = y;
                            if (w != "")
                                op_list[count_dpc].dp_ins[2] = w;
                            op_list[count_dpc].dp_outs[0] = z;
                            error = ew + ex + ey + ez;
                        }
                        count_dpc++;
                        if (error != 0) {
                            cout << endl
                                 << " Missing Variable" << endl;
                            system("pause");
                            return 2;
                        }
                    } else
                        op_list[sum_count_DPC].function = 2;
                    sum_count_DPC++;
                } else {
                    get_in_out_data(iline, str1, str2, str3, str4, bitsize);
                    instr[i] = str1;
                    if ((str1 != "") && (cir_list.inp_str != ""))
                        cir_list.inp_str = cir_list.inp_str + "," + str1;
                    else
                        cir_list.inp_str = cir_list.inp_str + str1;
                    outstr[i] = str2;
                    if ((str2 != "") && (cir_list.out_str != ""))
                        cir_list.out_str = cir_list.out_str + "," + str2;
                    else
                        cir_list.out_str = cir_list.out_str + str2;
                    varstr[i] = str3;
                    if ((str3 != "") && (cir_list.var_str != ""))
                        cir_list.var_str = cir_list.var_str + "," + str3;
                    else
                        cir_list.var_str = cir_list.var_str + str3;
                    sign_var[i] = stoi(str4);
                    DW[i] = bitsize;
                    i++;
                }
            }
        }
    } else {
        cout << "Unable to open file \n";
        system("pause");
        return 1;
    }
    op_num = sum_count_DPC;
    myfile1.close();

    parse_cir_inputs();
    asap_sequence(latency, error);
    alap_sequence(latency);
    op_prob();
    forceDSched();

    if (error == 4) {
        cout << "Incorrect Latency value \n";
        system("pause");
        return 4;
    }
    //get_sequence();

    /*filename2 = filename1.substr(0, filename1.find("."));
    filename2 = filename2 + ".v";*/
    ofstream myfile2(filename2); //open output file

    if (myfile2.is_open()) // open output file check and write to output file
    {
        oline = "`timescale 1ns / 1ns \n\n";
        oline = oline + "module HLSM (Clk, Rst, Start, ";
        for (m = 0; m < i; m++) {
            if (instr[m] != "") {
                oline = oline + instr[m];
            }
            if (outstr[m] != "") {
                oline = oline + outstr[m];
            }
            if ((instr[m + 1] != "") || (outstr[m + 1] != ""))
                oline = oline + ", ";
        }
        oline = oline + ", Done); \n";
        myfile2 << oline;
        myfile2 << "input Clk, Rst, Start; \n";
        myfile2 << "output reg Done; \n";

        for (u = 0; u < i; u++) {
            if (instr[u] != "") {
                bittemp = DW[u] - 1;
                if (sign_var[u] == 0)
                    newline = "input [" + std::to_string(bittemp + 1) + ":0] " + instr[u] + "; \n";
                else
                    newline = "input signed [" + std::to_string(bittemp) + ":0] " + instr[u] + "; \n";
                myfile2 << newline;
            }
            if (outstr[u] != "") {
                bittemp = DW[u] - 1;
                if (sign_var[u] == 0)
                    newline = "output reg [" + std::to_string(bittemp + 1) + ":0] " + outstr[u] + "; \n";
                else
                    newline = "output reg signed [" + std::to_string(bittemp) + ":0] " + outstr[u] + "; \n";
                myfile2 << newline;
            }
            if (varstr[u] != "") {
                bittemp = DW[u] - 1;
                if (sign_var[u] == 0)
                    newline = "reg [" + std::to_string(bittemp + 1) + ":0] " + varstr[u] + ", state; \n";
                else
                    newline = "reg signed [" + std::to_string(bittemp) + ":0] " + varstr[u] + ", state; \n";
                myfile2 << newline;
            }
        }

        myfile2 << "\nalways@(posedge Clk) begin \nif (Rst == 1'b1) begin \n\tstate <= 0; \n";
        m = 0;
        newline = "";
        while (cir_list.outs[m] != "") {
            newline = newline + "\t" + cir_list.outs[m] + " <= 0; \n";
            m++;
        }
        myfile2 << newline;
        myfile2 << "end else begin \n\tcase(state) \n";
        myfile2 << "\t\t0 : if (Start == 1'b1) begin\n\t\t\tstate <= 1;";
        myfile2 << "\n\t\t\tDone <= 0; \n";

        m = 0;
        newline = "";
        while (cir_list.outs[m] != "") {
            newline = newline + "\t\t\t" + cir_list.outs[m] + " <= 0; \n";
            m++;
        }
        myfile2 << newline;

        myfile2 << "\t\tend else begin \n\t\t\tstate <= 0; \n\t\tend";

        m = 1;
        oline = "";
        while (m <= latency) {
            myfile2 << "\n\t\t" << m << " : if (Start == 1'b1) begin\n\t\t\tstate <= " << m + 1 << ";\n";
            u = 0;
            oline = "";
            while (op_list[u].dp_outs_str != "") {
                if (op_list[u].FDSched == m) {
                    oline = oline + "\t\t\t" + op_list[u].dp_outs_str + ";\n";
                }
                u++;
            }
            if (oline != "")
                myfile2 << oline << "\t\t\tend";
            m++;
        }
        myfile2 << "\n\t\t" << m << " : if (Start == 1'b1) begin\n\t\t\tstate <= 0;"
                << "\n\t\t\tDone <= 1; \n\t\t\tend";
        myfile2 << "\n\t\tdefault : state <= 0; \n\tendcase \nend\nend ";
        myfile2 << "\nendmodule \n";
    } else {
        cout << "Unable to open file \n";
        system("pause");
        return 1;
    }
    myfile2.close();


    return 0;

}