// ECE574Assnm3.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

/*************************************************
1 - Get File Input subroutine
outputs string

2 - Parser subroutine
populates dataPath structure
elements include : original order, Resource type, input string, output string, sequence,
ALAP order, width, etc
populates variable to store Latency contraint, int lat_constraint;

3 - Error Checker subroutine

4 - Sequencing(not scheduled) subroutine
populates parts of dataPath structure : Sequence #, Predecssor List, Successor List,

5 - ASAP Schedule subroutine
populates parts of dataPath structure : ASAP order

6 - ALAP Schedule subroutine
populates parts of dataPath structure : ALAP order

7 - Width subroutine
Populates part of dataPath structure : width

8 - Operation Probability subroutine

creates int MUL_op[][lat_constraint]
creates int ALU_op[][lat_constraint]
creates int Logic_op[][lat_constraint]
creates int Div_op[][lat_constraint]

creates int ALU_dist[lat_constraint]
creates int MUL_dist[lat_constraint]
creates int Logic_dist[lat_constraint]
creates int Div_prob[lat_constraint]

9 - Total_Force subroutine
Calculate the total force for each dataPath operation
Total Force is the sum of self - force, predecessor force and successor force
populates totalForce element of dataPath structure

10 - Schedule subroutine
Populates FDS element of dataPath structure

11 - Write verilog file subroutine

switch stateReg
		case 1:			//clock cycle 1 operations

		case 2:			//clock cycle 2 operations

******************************************************/

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// ECE574Assnm3.cpp : This file contains the 'main' function. Program execution begins and ends there.

#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

//global variables

string ins_list = string();		//comma delimited list of inputs to the circuit
int circuit_clocks = 0;			//number of clock periods required to schedule this circuit
int l_cstrt = 7;				//latency constraint as specified by input
int op_num = 0;					//number of operations or node required by the input

//global structure contains attributes of Data Path Components (DPC)

struct dp_comp {				//Data Path Component (DPC) structure with attributes for DPC
	//populated by parsing routine
	int order;					//order as received from text file
	int function;				//enumeration of the components ( ALU = 0, MUL = 1, Logic/Logical = 2, Div = 3)
	string dp_ins_str;			//comma delimited list of inputs
	string dp_ins[2];			//array of inputs
	string dp_outs_str;			//comma delimited list of outputs
	string dp_outs[3];			//array of outputs
	//populated by sequencing routine
	int seq_clock;				//non-scheduled sequence order number
	int pred[2];
	int succ[2];
	//poplulate by ASAP routine
	int ASAP_clock;				//clock cycle for ASAP schedule
	//populate by ALAP routine
	int ALAP_clock;				//clock cycle for ALAP schedule
	//populate by width routine
	int width;					//time-frame width
	string out_line;			//output line to be sent to output file (verilog file)
	//populate by total force routine
	float t_force;				//output value of total force = self-force + predecessor force + successor force
	int FDSched;				//Force Direceted Schedule, time period in which operation is scheduled
	int low_f_period;			//period with lowest force for this operation
	//populate by self force routine
	float s_force;
	int op_loc;					//index of this node's data in the the specific resource probability table
	int FDSch;					//period in which node is schedule by FDS

	//delete later
	int i_clock;				//clock latency number
	int d_width;				//datpath width
	float latency;				//value from estimated latency table, used to find critical datapath
	int top_order;				//topological order used to find critical datapath

};
struct dp_comp op_list[20];	//create array of above structure

//global structure contains attributes of the Circuit
struct cir_desc {			//circuit description
	string inp_str;			//input string
	string ins[4];			//array of inputs to the circuit
	string inp_type;		//Int or UInt
	string out_str;
	string outs[4];			//array of outputs of the circuit
	string wires[4];
	string reg[4];
};
struct cir_desc cir_list;

double MUL_op[4][20];
double ALU_op[4][20];
double Logic_op[4][20];
double DIV_op[4][20];

double MUL_dist[20];	//MUL distribution
double ALU_dist[20];
double Logic_dist[20];
double DIV_dist[20];


//genaric routine to find input and output variables for datapath components

string iovalues(string &str, string &x, string &y, string &z) //find input and output variables for structure
{
	z = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	x = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	y = str.substr(0, str.find(" "));
	return (x, y, z);
}

//MUX  find the input and output variables for MUX

string iovaluesmux(string &str, string &w, string &x, string &y, string &z) //find input and output variables for structure
{
	z = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	w = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	x = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	y = str.substr(0, str.find(" "));
	return (x, y, z);
}

//REG  find the input and output variables for REG

string iovaluesreg(string &str, string &x, string &z) //find input and output variables for structure
{
	z = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	x = str.substr(0, str.find(" "));
	return (x, z);
}

//SHIFT  find the input and output variables for SHR and SHL

string iovaluesshift(string &str, string &x, string &y, string &z) //find input and output variables for structure
{
	z = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	x = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 4));
	y = str.substr(0, str.find(" "));
	return (x, y, z);
}

//COMP  find the input and output variables for COMP

string iovaluescomp(string &str, string &x, string &y, string &z) //find input and output variables for structure
{
	int tempinc;
	size_t yes;
	z = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	x = str.substr(0, str.find(" "));
	yes = (str.find("=="));
	if (yes != string::npos)
	{
		tempinc = 4;
	}
	else
	{
		tempinc = 3;
	}
	str = str.substr((str.find(" ") + tempinc));
	y = str.substr(0, str.find(" "));
	return (x, y, z);
}


int opcheck(string newline, string &x, string &z) //operation check
{
	int e;
	size_t h;
	string op;
	string ops[11] = { "+", "-", "*", "/", "%", "<", ">", "==", "<<", ">>", "?" };
	h = newline.find(" ");
	newline = newline.substr(h + 1);
	h = newline.find(" ");
	newline = newline.substr(h + 1);
	op = newline.substr(0, (newline.find(" ")));
	for (int b = 0; b < 12; b++)
	{
		if ((op != ops[b]) && (op != x) && (op != " "))
		{
			e = 1;
		}
		else
		{
			e = 0;
			break;
		}
	}
	return (e);
}

int varcheck(int &nx, int &ny, int &nz, string x, string y, string z, string str2, string str3, string str4, string str5) //variable check
{
	size_t v, q;
	string x1 = x + ",", x2 = x + " ";
	string y1 = y + ",", y2 = y + " ";
	string z1 = z + ",", z2 = z + " ";
	string s2 = str2 + ",", s3 = str3 + ",", s4 = str4 + ",", s5 = str5 + ",";
	v = s2.find(x1);
	q = s2.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s3.find(x1);
	q = s3.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s4.find(x1);
	q = s4.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s5.find(x1);
	q = s5.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s2.find(y1);
	q = s2.find(y2);
	if ((v != string::npos) || (q != string::npos))
	{
		ny++;
	}
	v = s3.find(y1);
	q = s3.find(y2);
	if ((v != string::npos) || (q != string::npos))
	{
		ny++;
	}
	v = s4.find(y1);
	q = s4.find(y2);
	if ((v != string::npos) || (q != string::npos))
	{
		ny++;
	}
	v = s5.find(y1);
	q = s5.find(y2);
	if ((v != string::npos) || (q != string::npos))
	{
		ny++;
	}
	v = s2.find(z1);
	q = s2.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	v = s3.find(z1);
	q = s3.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	v = s4.find(z1);
	q = s4.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	v = s5.find(z1);
	q = s5.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	return (0);
}

int varcheck2(int &nw, int &nx, int &ny, int &nz, string w, string x, string y, string z, string str2, string str3, string str4, string str5) //variable check
{

	string w1 = w + ",", w2 = w + " ";
	string x1 = x + ",", x2 = w + " ";
	string y1 = y + ",", y2 = w + " ";
	string z1 = z + ",", z2 = w + " ";
	string s2 = str2 + ",", s3 = str3 + ",", s4 = str4 + ",", s5 = str5 + ",";
	size_t v, q;
	v = s2.find(w1);
	q = s2.find(w2);
	if ((v != string::npos) || (q != string::npos))
	{
		nw++;
	}
	v = s3.find(w1);
	q = s3.find(w2);
	if ((v != string::npos) || (q != string::npos))
	{
		nw++;
	}
	v = s4.find(w1);
	q = s4.find(w2);
	if ((v != string::npos) || (q != string::npos))
	{
		nw++;
	}
	v = s5.find(w1);
	q = s5.find(w2);
	if ((v != string::npos) || (q != string::npos))
	{
		nw++;
	}
	v = s2.find(x1);
	q = s2.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s3.find(x1);
	q = s3.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s4.find(x1);
	q = s4.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s5.find(x1);
	q = s5.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s2.find(y1);
	q = s2.find(y2);
	if ((v != string::npos) || (q != string::npos))
	{
		ny++;
	}
	v = s3.find(y1);
	q = s3.find(y2);
	if ((v != string::npos) || (q != string::npos))
	{
		ny++;
	}
	v = s4.find(y1);
	q = s4.find(y2);
	if ((v != string::npos) || (q != string::npos))
	{
		ny++;
	}
	v = s5.find(y1);
	q = s5.find(y2);
	if ((v != string::npos) || (q != string::npos))
	{
		ny++;
	}
	v = s2.find(z1);
	q = s2.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	v = s3.find(z1);
	q = s3.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	v = s4.find(z1);
	q = s4.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	v = s5.find(z1);
	q = s5.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	return (0);
}
int varcheck3(int &nx, int &nz, string x, string z, string str2, string str3, string str4, string str5) //variable check
{

	size_t v, q;
	string x1 = x + ",", x2 = x + " ";
	string z1 = z + ",", z2 = z + " ";
	string s2 = str2 + ",", s3 = str3 + ",", s4 = str4 + ",", s5 = str5 + ",";
	v = s2.find(x1);
	q = s2.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s3.find(x1);
	q = s3.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s4.find(x1);
	q = s4.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s5.find(x1);
	q = s5.find(x2);
	if ((v != string::npos) || (q != string::npos))
	{
		nx++;
	}
	v = s2.find(z1);
	q = s2.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	v = s3.find(z1);
	q = s3.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	v = s4.find(z1);
	q = s4.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	v = s5.find(z1);
	q = s5.find(z2);
	if ((v != string::npos) || (q != string::npos))
	{
		nz++;
	}
	return (0);
}

void parse_cir_inputs(void)
{
	string temp_str = string();
	int p = 0;
	int findLoc = 0;
	int beg_pos = 0;
	int end_pos = 0;
	int index = 0;
	string var_str = string();

	temp_str = cir_list.inp_str;
	temp_str = temp_str + '\0';			//terminate with NULL

	end_pos = 0;
	p = -1;
	do {
		p++;
		//beg_pos = end_pos++;
		while (((temp_str[end_pos] == ' ') || (temp_str[end_pos] == ',')) && (temp_str[end_pos]))
		{
			end_pos++;			//step over spaces and comma to eliminate from variable
		}
		beg_pos = end_pos++;
		while (temp_str[end_pos] != ',' && (temp_str[end_pos]))
		{
			++end_pos;
		}
		cir_list.ins[p] = temp_str.substr(beg_pos, end_pos - beg_pos);
	} while (temp_str[end_pos] && p < 4);
}

//routine to find non-schedule sequence
//populate the seq_clock value of the structure for each operation
void get_sequence(void)
{
	int p = 0;								//counter used in for loop
	int n = 0;
	int k = 0;
	int inp_idx;							//index for inputs arrays
	int dpc_idx;							//index for data path components arrays
	int seq_clock = 1;						//clock periods used for scheduling
	int count_dpc_scheduled = 0;			//count the Data Path Components (DPC) scheduled
	string ready_inputs[15];				//array of inputs that are ready, allowing datapaths to be scheduled
	string dpc_inputs[12][10];				//DPC inputs [op_list.order][inp_index]
	string dpc_outputs[12][10];				//DPC inputs [op_list.order][inp_index]
	string temp_str = string();				//temporary string
	int beg_pos = 0;						//string index
	int end_pos = 0;
	int count_dpc = 0;						//count the components
	int ready_ins_count;					//number of inputs in ready_inputs list
	int dpc_item_count = 0;					//number of DPC in circuit
	int dpc_schd = 0;						//count dpc scheduled
	int rdy_idx = 0;
	int inp_found = 0;						//flag inputs as found in ready input list

	cout << "get_sequence" << endl;
	parse_cir_inputs();

	cout << cir_list.inp_str << endl;
	cout << "output string" << cir_list.out_str << endl;

	cout << "circuit inputs:  " << endl;
	for (p = 0; p < 3; ++p) {
		cout << cir_list.ins[p] << endl;
	}
	cout << endl << endl;

	cout << "ready_inputs:  " << endl;
	for (p = 0; p < 3; ++p) {
		ready_inputs[p] = cir_list.ins[p];		//copy circuit inpputs
		cout << ready_inputs[p] << ", ";
	}
	cout << endl;

	for (p = 4; p < 15; p++) {
		ready_inputs[p] = string();		//empty string
	}

	ready_ins_count = 0;
	do {
		ready_ins_count++;				//count the number of ready inpuuts
	//} while (!ready_inputs[ready_ins_count].empty());
	} while (ready_inputs[ready_ins_count] != "\0");
	cout << "ready inputs:  " << ready_ins_count << endl;

	//DPC that can run in first clock period have inputs in the circuit inputs list
	//schedule the components for the first clock period
	dpc_idx = 0;
	while (op_list[dpc_idx].order) {			//check every dpc against the circuit inputs
		if ((op_list[dpc_idx].dp_ins[0] == cir_list.ins[0]) || (op_list[dpc_idx].dp_ins[0] == cir_list.ins[1]) ||
			(op_list[dpc_idx].dp_ins[0] == cir_list.ins[2]) || (op_list[dpc_idx].dp_ins[3] == cir_list.ins[3]))
		{
			op_list[dpc_idx].seq_clock = seq_clock;			//schedule the dpc
			cout << ".seq_clock  " << dpc_idx << ": " << op_list[p].seq_clock << endl;

			p = 0;											//add datapath outputs to ready_ouputs array
			do {											//all dpc have at least 1 input
				ready_inputs[ready_ins_count - 1] = cir_list.outs[p];
				p++;
				//} while( ! cir_list.ins[p].empty() );
			} while (!cir_list.ins[p].empty());
		}
		dpc_idx++;
	}

	//check every DPC against the ready inputs list; then increment the clock
	//dpc_idx = 0;
	do {						//clock loop
		seq_clock++;				//advance to next clock period
		dpc_idx = 0;
		do {										//dpc loop
			if (!op_list[dpc_idx].seq_clock)			//if this DPC is NOT scheduled, compare inputs to ready_inputs
			{
				inp_idx = 0;
				do {								//dpc inputs loop
					rdy_idx = 0;
					inp_found = 0;
					do {					//ready inputs loop
						if (op_list[dpc_idx].dp_ins[inp_idx] == ready_inputs[rdy_idx]) {
							inp_found = 1;
						}
						else {
							rdy_idx++;
						}
					} while (!inp_found && !ready_inputs[rdy_idx].empty());		//until input is found or no more ready inputs
					inp_idx++;			//advance to next input
				} while (inp_found && !op_list[dpc_idx].dp_ins[inp_idx].empty());			//until all inputs are checked or an input is not found
				if (inp_found && !op_list[dpc_idx].dp_ins[inp_idx].empty())
				{
					op_list[dpc_idx].seq_clock = seq_clock;			//schedule the DPC for this clock period
					dpc_schd++;										//count dpc scheduled
				}
			}  //if (!op_list[dpc_idx].seq_clock)
			dpc_idx++;			//advance to next dpc
		} while (op_list[dpc_idx].order);						//.order is zero if array is empty	
	} while (dpc_schd < dpc_item_count);			//repeat until all components have been scheduled

	circuit_clocks = seq_clock;			//number of clock cycles required for this circuit
	for (p = 0; p < dpc_item_count; p++) {
		cout << "p i.clock:  " << p << "  " << op_list[p].seq_clock << endl;
	}
}

//calculate the probability for each operation in each clock period
void op_prob(void)
{

	int mul_count = 0;		//counters for respective operations
	int alu_count = 0;
	int logic_count = 0;
	int div_count = 0;
	int p = 0;				//count operations
	int clk_count = 0;		//count clock periods

	//inialize array variable to zero
	for (clk_count = 0; clk_count < 20; ++clk_count)
	{
		for (p = 0; p < 4; ++p)
		{
			MUL_op[p][clk_count] =0;
			ALU_op[p][clk_count] =0;
			Logic_op[p][clk_count] =0;
			DIV_op[p][clk_count] =0;
		}
		MUL_dist[clk_count] = 0;
		ALU_dist[clk_count] = 0;
		Logic_dist[clk_count] = 0;
		DIV_dist[clk_count] = 0;
	}

	do {
		switch( op_list[p].function)		// (ALU = 0, MUL = 1, Logic / Logical = 2, Div = 3)
		{
			case 0:
				for (clk_count = 0; clk_count < l_cstrt; clk_count++)
				{
					MUL_op[mul_count][clk_count] = 1.0 / op_list[p].width;
					op_list[p].op_loc = mul_count;
				}
				mul_count++;											//count multiplier operations
				break;
			case 1:
				for (clk_count = 0; clk_count < l_cstrt; clk_count++)
				{
					ALU_op[alu_count][clk_count] = 1.0 / op_list[p].width;
					op_list[p].op_loc = alu_count;
				}
				alu_count++;
				break;

			case 2:
				for (clk_count = 0; clk_count < l_cstrt; clk_count++)
				{
					Logic_op[logic_count][clk_count] = 1.0 / op_list[p].width;
					op_list[p].op_loc = logic_count;
					}
				logic_count++;
				break;

			case 3:
				for (clk_count = 0; clk_count < l_cstrt; clk_count++)
				{
					DIV_op[div_count][clk_count] = 1.0 / op_list[p].width;
					op_list[p].op_loc = div_count;
				}
				div_count++;
				break;
		} //switch
		++p;
	} while (op_list[p].function < 99);

	//sum the probabilities for each clock period
	//MUL
	for (clk_count = 0; clk_count < l_cstrt; clk_count++)
	{
		MUL_dist[clk_count] = MUL_op[0][clk_count] + MUL_op[1][clk_count] + MUL_op[2][clk_count] + MUL_op[3][clk_count];
	}
	//ALU
	for (clk_count = 0; clk_count < l_cstrt; clk_count++)
	{
		ALU_dist[clk_count] = ALU_op[0][clk_count] + ALU_op[1][clk_count] + ALU_op[2][clk_count] + ALU_op[3][clk_count];
	}
	//Logic
	for (clk_count = 0; clk_count < l_cstrt; clk_count++)
	{
		Logic_dist[clk_count] = Logic_op[0][clk_count] + Logic_op[1][clk_count] + Logic_op[2][clk_count] + Logic_op[3][clk_count];
	}
	//DIV
	for (clk_count = 0; clk_count < l_cstrt; clk_count++)
	{
		DIV_dist[clk_count] = DIV_op[0][clk_count] + DIV_op[1][clk_count] + DIV_op[2][clk_count] + DIV_op[3][clk_count];
	}
}

//given the node number and the clock period 
//this function calculates the self-force
double calcSelfForce(int node, int t_period)
{
	int k = 0;				//index clock periods
	int fun_type = 0;		//type of function from 
	double self_force = 0;	//self force calculated, return this value
	double temp_force = 0;	//temporary local variable

	fun_type = op_list[node].function;			// (ALU = 0, MUL = 1, Logic / Logical = 2, Div = 3)

	switch (fun_type)
		{
		case 0:									//ALU
			for (k = 0; k < l_cstrt; ++k)
			{
				if (t_period == k + 1)			//time period count starts at 1
					temp_force = (ALU_dist[k]) * (1 - ALU_op[op_list[node].op_loc][k]);		//time period under consideration
				else
					temp_force = (ALU_dist[k]) * (0 - ALU_op[op_list[node].op_loc][k]);		//not time period under consideration
				self_force = self_force + temp_force;
			}
			break;
		case 1:									//MUL
			for (k = 0; k < l_cstrt; ++k)
			{
				if (t_period == k + 1)
					temp_force = (MUL_dist[k]) * (1 - MUL_op[op_list[node].op_loc][k]);		//time period under consideration
				else
					temp_force = (MUL_dist[k]) * (0 - MUL_op[op_list[node].op_loc][k]);		//not time period under consideration
				self_force = self_force + temp_force;
			}
			break;
		case 2:									//Logic
			for (k = 0; k < l_cstrt; ++k)
			{
				if (t_period == k + 1)
					temp_force = (Logic_dist[k]) * (1 - Logic_op[op_list[node].op_loc][k]);		//time period under consideration
				else
					temp_force = (Logic_dist[k]) * (0 - Logic_op[op_list[node].op_loc][k]);		//not time period under consideration
				self_force = self_force + temp_force;
			}
			break;
		case 3:									//DIV
			for (k = 0; k < l_cstrt; ++k)
			{
				if (t_period == k + 1)
					temp_force = (DIV_dist[k]) * (1 - DIV_op[op_list[node].op_loc][k]);		//time period under consideration
				else
					temp_force = (DIV_dist[k]) * (0 - DIV_op[op_list[node].op_loc][k]);		//not time period under consideration
				self_force = self_force + temp_force;
			}
			break;
		}

	return self_force;
}

float calcTForce(int node, int t_period) 
{
	int p = 0;
	int ps_node1 = 0;
	int ps_node2 = 0;
	double self_force = 0;
	double pred_force1 = 0;
	double pred_force2 = 0;
	double succ_force1 = 0;
	double succ_force2 = 0;
	double total_force = 0;

	self_force = calcSelfForce(node, t_period);
	
	//find predecessor forces
	if (t_period != 1)						//must find predecessor forces when not in time period 1
	{
		ps_node1 = op_list[node].pred[0];
		if ((t_period - 1) == op_list[ps_node1].ASAP_clock)
		{
			pred_force1 = calcSelfForce(ps_node1, t_period - 1);
		}
		ps_node2 = op_list[node].pred[1];
		if ((t_period - 1) == op_list[ps_node2].ASAP_clock)
		{
			pred_force2 = calcSelfForce(ps_node2, t_period - 1);
		}
	}

	//find successor forces
	if (t_period != l_cstrt)						//must find successor forces when not in last time period
	{
		ps_node1 = op_list[node].succ[0];
		if ((t_period + 1) == op_list[ps_node1].ASAP_clock)
		{
			succ_force1 = calcSelfForce(ps_node1, t_period + 1);
		}
		ps_node2 = op_list[node].succ[1];
		if ((t_period + 1) == op_list[ps_node2].ASAP_clock)
		{
			succ_force2 = calcSelfForce(ps_node2, t_period + 1);
		}
	}

	total_force = self_force + pred_force1 + pred_force2 + succ_force1 + succ_force2;

	return total_force;
}

void forceDSched(void)
{
	int sched_count = 0;							//count operations scheduled
	int p = 0;
	int k = 0;
	int low_node = 0;								//node with least force
	double low_force = 0;							//least force
	int low_period = 0;								//period with least force, used with low node
	double forces[20][10];					//array of forces, number of operations x latency constraint

	//first step is to schedule implicitly scheduled operations; operations with a width of 1
	for (p = 0; p < op_num; ++p)
	{
		if (op_list[p].width == 1)
		{
			op_list[p].FDSch = op_list[p].ASAP_clock;
			sched_count++;							//count operations scheduled
		}
	}

	while (sched_count < op_num)					//until all operations are scheduled
	{		
		//initialize all forces to bogus high value
		for (p = 0; p < op_num; ++p)				//every node
		{
			for (k = 0; k < l_cstrt; ++k)			//every clock period
			{
				forces[p][k] = 999.9;				//p is node, k is time period
			}
		}

		//find self force for each operation within its time frame
		for (p = 0; p < op_num; ++p)		//every node
		{
			if (!op_list[p].FDSch)			//not scheduled yet
			{
				for (k = op_list[p].ASAP_clock; k < op_list[p].ALAP_clock + 1; ++k)
				{
					//forces[p][k] = calcSelfForce(p, k);				//p is node, k is time period
					forces[p][k] = calcTForce(p, k);				//p is node, k is time period
				}
			}
		}

		low_force = 99.9;					//start with bogus high value
		//find the operation with the least force
		for (p = 0; p < op_num; ++p)		//every node
		{
			for (k = 0; k < l_cstrt; ++k)	//every clock period
			{
				if (!op_list[p].FDSch)		//not scheduled yet
				{
					if (forces[p][k] < low_force)
					{
						low_force = forces[p][k];		//update low force
						low_node = p;					//update low node
						low_period = k;					//update low period
					}
				}
			}
		}

		op_list[p].FDSched = op_list[p].low_f_period;			//schedule this operation for its lowest force period
		op_list[p].ASAP_clock = op_list[p].FDSched;				//update the time frame
		op_list[p].ALAP_clock = op_list[p].FDSched;
		sched_count++;											//count operations scheduled
	} //while (sched_count < op_num)						//until all operations are scheduled
}

int main()
{
	string filename, filename1, filename2, iline, oline, newline, str, str1, strc, strv;
	string str2, str3, str4, str5;
	string instr[10] = {}, outstr[10] = {}, wirestr[10] = {}, regstr[12] = {};
	string insize, outsize, w, x, y, z, w_dw[14] = {}, x_dw[14] = {}, y_dw[14] = {}, z_dw[14] = {};
	size_t found, found1, found2, found3, found4, found5, found6;
	size_t found7, found8, found9, found10, found11, foundname1, foundname2;
	int bittemp, temp = 0, bitsize = 0, start = 0, i = 0, m = 0, DW[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	//array to count instances of each datapath component
	int s = 0, error = 0, here = 0, count_DPC[12] = { 0,0,0,0,0,0,0,0,0,0,0,0 };
	int nw = 0, nx = 0, ny = 0, nz = 0, sw = 0, sx = 0, sy = 0, sz = 0;
	int u = 0, sign_var[10] = { 0,0,0,0,0,0,0,0,0,0 };
	int sum_count_DPC = 0;		//count the datapath components, which is sum of count_DPC[]
	int c_period_req = 0;		//number of clock periods required by schedule
	float cr_dp = 0.0;			//critical data path

//initialize the cir_list array
	for (i = 0; i < 4; i++) {
		cir_list.ins[i] = "\0";		//array of inputs to the circuit	
		cir_list.outs[i] = string();		//array of outputs of the circuit
		cir_list.wires[i] = string();
		cir_list.reg[i] = string();
	}
	cir_list.inp_str = "\0";

	//initailize the op_list array
	for (i = 0; i < 20; i++) {
		op_list[i].function = 99;		//component enumeration uses 0 thru 12
		op_list[i].order = 0;
		op_list[i].top_order = 0;
		op_list[i].out_line = string();
		op_list[i].d_width = 0;
		op_list[i].latency = 0.0;
		op_list[i].i_clock = 0;
		op_list[i].dp_ins[0] = string();
		op_list[i].dp_ins[1] = string();
		op_list[i].dp_ins_str = string();
		op_list[i].dp_outs[0] = string();
		op_list[i].dp_outs[1] = string();
		op_list[i].dp_outs[2] = string();
		op_list[i].dp_outs_str = string();
		op_list[i].FDSched = 0;				//time period in which operation is scheduled, zero for NOT scheduled, first time period is 1
		op_list[i].low_f_period = 0;
		op_list[i].pred[0] = 0;
		op_list[i].pred[1] = 0;
		op_list[i].succ[0] = 0;
		op_list[i].succ[1] = 0;
	}
	i = 0;

	cout << "Please enter filename: "; // generate output file 
	cin >> filename;
	filename1 = filename + ".txt";
	filename2 = filename + ".v";

	ifstream myfile1(filename1); // open input file
	ofstream myfile2(filename2); //open output file

	if (myfile1.is_open()) // open input file check and write to output file check
	{
		oline = "`timescale 1ns / 1ns \n";
		if (myfile2.is_open())
		{
			myfile2 << oline << '\n';
		}
		else cout << "Unable to open file";
	}
	else cout << "Unable to open file";

	/***************
	This while loop block separates the input file into 4 types of strings:
	inputs, outputs, wires and register strings.
	it determines the DATAWIDTH for the module
	and writes the first line of the module's code that identifies the module, DATAWIDTH, and variables
	******************/

	while (getline(myfile1, iline)) //parse the input variables
	{
		foundname1 = iline.find("input");
		if (foundname1 != string::npos)
		{
			if ((foundname1 = iline.find("UInt")) != string::npos) {
				iline = iline.substr(foundname1 + 4);
				cir_list.inp_type = "UInt";
				u++;
			}
			if ((foundname1 = iline.find("Int")) != string::npos) {
				iline = iline.substr(foundname1 + 3);
				cir_list.inp_type = "Int";
				sign_var[u] = 1;
				u++;
			}
			foundname2 = iline.find(" ");
			if (foundname2 != string::npos)
			{
				insize = iline.substr(0, foundname2);
				bittemp = stoi(insize);
				if (bittemp < bitsize)
					bitsize = bitsize;
				else
					bitsize = bittemp;
			}
			instr[m] = iline.substr(foundname2 + 1);
			cout << "instr[m]" << m << instr[m] << endl;
			m++;
			DW[i] = bittemp;
			i = i++;
		}
		foundname1 = iline.find("output"); // parse the output variables
		if (foundname1 != string::npos)
		{
			if ((foundname1 = iline.find("UInt")) != string::npos)
			{
				iline = iline.substr(foundname1 + 4);
				u++;
			}
			if ((foundname1 = iline.find("Int")) != string::npos)
			{
				iline = iline.substr(foundname1 + 3);
				sign_var[u] = 1;
				u++;
			}
			foundname2 = iline.find(" ");
			if (foundname2 != string::npos)
			{
				outsize = iline.substr(0, foundname2);
				bittemp = stoi(outsize);
				if (bittemp < bitsize)
					bitsize = bitsize;
				else
					bitsize = bittemp;
			}
			outstr[m] = iline.substr(foundname2);

			//ins_list = ins_list + instr[m] + " ,"
			m++;
			DW[i] = bittemp;
			i = i++;
		}

		foundname1 = iline.find("wire");
		if ((foundname1 != string::npos) && (foundname1 == 0))
		{
			if ((foundname1 = iline.find("UInt")) != string::npos)
			{
				iline = iline.substr(foundname1 + 4);
				u++;
			}
			if ((foundname1 = iline.find("Int")) != string::npos)
			{
				iline = iline.substr(foundname1 + 3);
				sign_var[u] = 1;
				u++;
			}
			foundname2 = iline.find(" ");
			if (foundname2 != string::npos)
			{
				insize = iline.substr(0, foundname2);
				bittemp = stoi(insize);
			}
			wirestr[m] = iline.substr(foundname2);
			m++;
			DW[i] = bittemp;
			i = i++;
		}

		foundname1 = iline.find("register");
		if (foundname1 != string::npos)
		{
			if ((foundname1 = iline.find("UInt")) != string::npos)
			{
				iline = iline.substr(foundname1 + 4);
				u++;
			}
			if ((foundname1 = iline.find("Int")) != string::npos)
			{
				iline = iline.substr(foundname1 + 3);
				sign_var[u] = 1;
				u++;
			}
			foundname2 = iline.find(" ");
			if (foundname2 != string::npos)
			{
				insize = iline.substr(0, foundname2);
				bittemp = stoi(insize);
			}
			regstr[m] = iline.substr(foundname2);
			m++;
			DW[i] = bittemp;
			i = i++;
		}
	}
	for (int c = 0; c < i; c++)
	{
		if (instr[c] != "")
		{
			if (c == 0)
			{
				str = instr[c];
			}
			else
			{
				str = str + "," + instr[c];
			}
		}
		if (outstr[c] != "")
		{
			str = str + "," + outstr[c];
		}
	}
	oline = "module " + filename + " #(parameter DATAWIDTH = " + std::to_string(bitsize) + ")(" + str + "); \n";
	myfile2 << oline;

	/**************
	This for loop block creates declartions of inputs, outputs, wires and registers
	***************/
	int inpt_count = 0;
	int outpt_count = 0;
	int wire_count = 0;
	int reg_count = 0;

	for (int p = 0; p < i; p++)
	{
		if (instr[p] != "")
		{
			str = instr[p];
			if ((stoi(outsize)) >= (DW[p]))
			{
				string str2 = std::to_string(bitsize / (DW[p]));
				if (sign_var[p] == 0)
					newline = newline + "input [DATAWIDTH/" + str2 + "-1:0]" + str + "; \n";
				if (sign_var[p] > 0)
					newline = newline + "input signed [DATAWIDTH/" + str2 + "-1:0]" + str + "; \n";
			}
			else
			{
				string str2 = std::to_string((DW[p]) / bitsize);
				if (sign_var[p] == 0)
					newline = newline + "input [DATAWIDTH*" + str2 + "-1:0]" + str + "; \n";
				if (sign_var[p] > 0)
					newline = newline + "input signed [DATAWIDTH*" + str2 + "-1:0]" + str + "; \n";
			}
			cir_list.inp_str = str;
			inpt_count++;
		}
		if (outstr[p] != "")
		{
			str = outstr[p];
			if ((stoi(outsize)) >= (DW[p]))
			{
				string str2 = std::to_string(bitsize / (DW[p]));
				if (sign_var[p] == 0)
					newline = newline + "output wire [DATAWIDTH/" + str2 + "-1:0]" + str + "; \n";
				if (sign_var[p] > 0)
					newline = newline + "output wire signed [DATAWIDTH/" + str2 + "-1:0]" + str + "; \n";
			}

			else
			{
				string str2 = std::to_string((DW[p]) / bitsize);
				if (sign_var[p] == 0)
					newline = newline + "output wire [DATAWIDTH*" + str2 + "-1:0]" + str + "; \n";
				if (sign_var[p] > 0)
					newline = newline + "output wire signed [DATAWIDTH*" + str2 + "-1:0]" + str + "; \n";
			}
			cir_list.out_str = str;
			outpt_count++;
		}
		if (wirestr[p] != "")
		{
			str = wirestr[p];
			if ((stoi(outsize)) >= (DW[p]))
			{
				string str2 = std::to_string(bitsize / (DW[p]));
				if (sign_var[p] == 0)
					newline = newline + "wire [DATAWIDTH/" + str2 + "-1:0]" + str + "; \n";
				if (sign_var[p] > 0)
					newline = newline + "wire signed [DATAWIDTH/" + str2 + "-1:0]" + str + "; \n";
			}

			else
			{
				string str2 = std::to_string((DW[p]) / bitsize);
				if (sign_var[p] == 0)
					newline = newline + "wire [DATAWIDTH*" + str2 + "-1:0]" + str + "; \n";
				if (sign_var[p] > 0)
					newline = newline + "wire signed [DATAWIDTH*" + str2 + "-1:0]" + str + "; \n";
			}
			cir_list.wires[wire_count] = str;
		}
		if (regstr[p] != "")
		{
			str = regstr[p];
			if ((stoi(outsize)) >= (DW[p]))
			{
				string str2 = std::to_string(bitsize / (DW[p]));
				if (sign_var[p] == 0)
					newline = newline + "register [DATAWIDTH/" + str2 + "-1:0]" + str + "; \n";
				if (sign_var[p] > 0)
					newline = newline + "register signed [DATAWIDTH/" + str2 + "-1:0]" + str + "; \n";
			}
			else
			{
				string str2 = std::to_string((DW[p]) / bitsize);
				if (sign_var[p] == 0)
					newline = newline + "register [DATAWIDTH*" + str2 + "-1:0]" + str + "; \n";
				if (sign_var[p] > 0)
					newline = newline + "register signed [DATAWIDTH*" + str2 + "-1:0]" + str + "; \n";
			}
			cir_list.reg[reg_count] = str;
			reg_count++;
		}
	}
	oline = "";
	myfile1.close();
	myfile2 << newline;
	ifstream myfile3(filename1);

	if (myfile3.is_open()) // open input file check and write to output file check
	{
		myfile2 << '\n';
	}
	else cout << "Unable to open file";

	/**********************************
	this while loop bloc works through the input file looking for datapath components
	and writes a line to the output file for each datapath component
	***********************************/
	int count_dpc = 0;					//count the datapath components found
	while (getline(myfile3, iline))
	{
		found = iline.find(" = "); // detemine which operation is being performed and generate structure line to output
		if ((found != string::npos) && (found < 50))
		{
			temp = 0;
			newline = iline.substr(found + 2);
			found1 = iline.find(" + ");					//select ADD or INC
			if (found1 != string::npos)
			{
				temp = 1;
				found1 = iline.find("+ 1");
				if (found1 != string::npos)
				{
					str = iline;
					(x, y, z) = iovalues(str, x, y, z);
					for (int m = 0; m < i; m++)
					{
						str2 = instr[m];
						str3 = outstr[m];
						str4 = wirestr[m];
						str5 = regstr[m];
						s = varcheck3(nx, nz, x, z, str2, str3, str4, str5);
						//here = here + s;
						if ((nx > 0) && (nz > 0))
							break;
					}
					if ((nx == 0) || (nz == 0))
					{
						cout << endl
							<< " Missing Variable " << endl;
						break;
					}
					here = 0;
					nw = 0, nx = 0, ny = 0, nz = 0;
					for (int g = 0; g < i; g++)
					{
						strc = instr[g];
						if (strc.find(z) < 50)
						{
							z_dw[0] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = outstr[g];
						if (strc.find(z) < 50)
						{
							z_dw[0] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = wirestr[g];
						if (strc.find(z) < 50)
						{
							z_dw[0] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = regstr[g];
						if (strc.find(z) < 50)
						{
							z_dw[0] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
					}
					for (int l = 0; l < i; l++)
					{
						strc = instr[l];
						if (strc.find(x) < 50)
						{
							x_dw[0] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = outstr[l];
						if (strc.find(x) < 50)
						{
							x_dw[0] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = wirestr[l];
						if (strc.find(x) < 50)
						{
							x_dw[0] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = regstr[l];
						if (strc.find(x) < 50)
						{
							x_dw[0] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
					}

					count_DPC[10]++;							//count instances of INC
					if ((sx == 0) && (sz == 0))
						oline = oline + "INC #(.DATAWIDTH" + z_dw[0] + ") INC_" + std::to_string(count_DPC[10]) + "(.a(" + x + "), .d(" + z + ")); \n";
					else
					{
						if (sx == 0)
						{
							x = "$signed({1'b0," + x + "}";
						}
						if (sz == 0)
						{
							z = "$signed({1'b0," + z + "}";
						}
						oline = oline + "SINC #(.DATAWIDTH" + z_dw[0] + ") SINC_" + std::to_string(count_DPC[10]) + "(.a(" + x + "), .d(" + z + ")); \n";
					}

					op_list[count_dpc].dp_ins[0] = x;
					op_list[count_dpc].dp_ins[1] = x;
					op_list[count_dpc].dp_outs[0] = z;
					op_list[sum_count_DPC].function = 10;
					op_list[sum_count_DPC].order = sum_count_DPC + 1;
					op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
					//op_list[sum_count_DPC].latency = std::stod(strv);
					op_list[sum_count_DPC].out_line = oline;
					op_list[sum_count_DPC].d_width = std::stoi(z_dw[0]);
					sum_count_DPC++;
				}
				else
				{
					str = iline;
					(x, y, z) = iovalues(str, x, y, z);
					for (int m = 0; m < i; m++)
					{
						str2 = instr[m];
						str3 = outstr[m];
						str4 = wirestr[m];
						str5 = regstr[m];
						s = varcheck(nx, ny, nz, x, y, z, str2, str3, str4, str5);
						//here = here + s;
						if ((nx > 0) && (ny > 0) && (nz > 0))
							break;
					}
					if ((nx == 0) || (ny == 0) || (nz == 0))
					{
						cout << endl
							<< " Missing Variable " << endl;
						break;
					}
					here = 0;
					nw = 0, nx = 0, ny = 0, nz = 0;
					for (int g = 0; g < i; g++)
					{
						strc = instr[g];
						if (strc.find(z) < 50)
						{
							z_dw[1] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = outstr[g];
						if (strc.find(z) < 50)
						{
							z_dw[1] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = wirestr[g];
						if (strc.find(z) < 50)
						{
							z_dw[1] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = regstr[g];
						if (strc.find(z) < 50)
						{
							z_dw[1] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
					}
					for (int l = 0; l < i; l++)
					{
						strc = instr[l];
						if (strc.find(x) < 50)
						{
							x_dw[1] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = outstr[l];
						if (strc.find(x) < 50)
						{
							x_dw[1] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = wirestr[l];
						if (strc.find(x) < 50)
						{
							x_dw[1] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = regstr[l];
						if (strc.find(x) < 50)
						{
							x_dw[1] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
					}
					for (int k = 0; k < i; k++)
					{
						strc = instr[k];
						if (strc.find(y) < 50)
						{
							y_dw[1] = std::to_string(DW[k]);
							sy = sign_var[k];
							break;
						}
						strc = outstr[k];
						if (strc.find(y) < 50)
						{
							y_dw[1] = std::to_string(DW[k]);
							sy = sign_var[k];
							break;
						}
						strc = wirestr[k];
						if (strc.find(y) < 50)
						{
							y_dw[1] = std::to_string(DW[k]);
							sy = sign_var[k];
							break;
						}
						strc = regstr[k];
						if (strc.find(y) < 50)
						{
							y_dw[1] = std::to_string(DW[k]);
							sy = sign_var[k];
							break;
						}
					}
					count_DPC[1]++;							//count instances of ADD
					if ((sx == 0) && (sy == 0) && (sz == 0))
						oline = oline + "ADD #(.DATAWIDTH(" + z_dw[1] + ")) ADD_" + std::to_string(count_DPC[1]) + "(.a(" + x + "), .b(" + y + "), .sum(" + z + ")); \n";
					else
					{
						if (sx == 0)
						{
							x = "$signed({1'b0," + x + "}";
						}
						if (sy == 0)
						{
							y = "$signed({1'b0," + y + "}";
						}
						if (sz == 0)
						{
							z = "$signed({1'b0," + z + "}";
						}
						oline = oline + "SADD #(.DATAWIDTH(" + z_dw[1] + ")) SADD_" + std::to_string(count_DPC[1]) + "(.a(" + x + "), .b(" + y + "), .sum(" + z + ")); \n";
					}

					op_list[sum_count_DPC].dp_ins[0] = x;
					op_list[sum_count_DPC].dp_ins[1] = y;
					op_list[sum_count_DPC].dp_outs[0] = z;
					op_list[sum_count_DPC].function = 1;
					op_list[sum_count_DPC].order = sum_count_DPC + 1;
					op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
					//op_list[sum_count_DPC].latency = std::stof(strv);
					op_list[sum_count_DPC].out_line = oline;
					op_list[sum_count_DPC].d_width = std::stoi(z_dw[1]);
					sum_count_DPC++;
				}
			}
			found2 = iline.find(" - ");					//select SUB or DEC
			if (found2 != string::npos)
			{
				temp = 2;
				found2 = iline.find("- 1");
				if (found2 != string::npos)
				{
					str = iline;
					(x, y, z) = iovaluesreg(str, x, z);
					for (int m = 0; m < i; m++)
					{
						str2 = instr[m];
						str3 = outstr[m];
						str4 = wirestr[m];
						str5 = regstr[m];
						s = varcheck3(nx, nz, x, z, str2, str3, str4, str5);
						//here = here + s;
						if ((nx > 0) && (nz > 0))
							break;
					}
					if ((nx == 0) || (nz == 0))
					{
						cout << endl
							<< " Missing Variable " << endl;
						break;
					}
					here = 0;
					nw = 0, nx = 0, ny = 0, nz = 0;
					for (int g = 0; g < i; g++)
					{
						strc = instr[g];
						if (strc.find(z) < 50)
						{
							z_dw[2] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = outstr[g];
						if (strc.find(z) < 50)
						{
							z_dw[2] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = wirestr[g];
						if (strc.find(z) < 50)
						{
							z_dw[2] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = regstr[g];
						if (strc.find(z) < 50)
						{
							z_dw[2] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
					}
					for (int l = 0; l < i; l++)
					{
						strc = instr[l];
						if (strc.find(x) < 50)
						{
							x_dw[2] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = outstr[l];
						if (strc.find(x) < 50)
						{
							x_dw[2] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = wirestr[l];
						if (strc.find(x) < 50)
						{
							x_dw[2] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = regstr[l];
						if (strc.find(x) < 50)
						{
							x_dw[2] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
					}
					count_DPC[11]++;							//count instances of DEC
					if ((sx == 0) && (sz == 0))
						oline = oline + "DEC #(.DATAWIDTH(" + z_dw[2] + ")) DEC_" + std::to_string(count_DPC[11]) + "(.a(" + x + "), .d(" + z + ")); \n";
					else
					{
						if (sx == 0)
						{
							x = "$signed({1'b0," + x + "}";
						}
						if (sz == 0)
						{
							z = "$signed({1'b0," + z + "}";
						}
						oline = oline + "SDEC #(.DATAWIDTH(" + z_dw[2] + ")) SDEC_" + std::to_string(count_DPC[11]) + "(.a(" + x + "), .d(" + z + ")); \n";
					}

					op_list[sum_count_DPC].dp_ins[0] = x;
					//op_list[count_dpc].dp_ins[1] = y;
					op_list[sum_count_DPC].dp_outs[0] = z;
					op_list[sum_count_DPC].function = 11;
					op_list[sum_count_DPC].order = sum_count_DPC + 1;
					op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
					//op_list[sum_count_DPC].latency = std::stof(strv);
					op_list[sum_count_DPC].out_line = oline;
					op_list[sum_count_DPC].d_width = std::stoi(z_dw[2]);
					sum_count_DPC++;

				}
				else
				{
					str = iline;
					(x, y, z) = iovalues(str, x, y, z);
					for (int m = 0; m < i; m++)
					{
						str2 = instr[m];
						str3 = outstr[m];
						str4 = wirestr[m];
						str5 = regstr[m];
						s = varcheck(nx, ny, nz, x, y, z, str2, str3, str4, str5);
						//here = here + s;
						if ((nx > 0) && (ny > 0) && (nz > 0))
							break;
					}
					if ((nx == 0) || (ny == 0) || (nz == 0))
					{
						cout << endl
							<< " Missing Variable " << endl;
						break;
					}
					here = 0;
					nw = 0, nx = 0, ny = 0, nz = 0;
					for (int g = 0; g < i; g++)
					{
						strc = instr[g];
						if (strc.find(z) < 50)
						{
							z_dw[3] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = outstr[g];
						if (strc.find(z) < 50)
						{
							z_dw[3] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = wirestr[g];
						if (strc.find(z) < 50)
						{
							z_dw[3] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
						strc = regstr[g];
						if (strc.find(z) < 50)
						{
							z_dw[3] = std::to_string(DW[g]);
							sz = sign_var[g];
							break;
						}
					}
					for (int l = 0; l < i; l++)
					{
						strc = instr[l];
						if (strc.find(x) < 50)
						{
							x_dw[3] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = outstr[l];
						if (strc.find(x) < 50)
						{
							x_dw[3] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = wirestr[l];
						if (strc.find(x) < 50)
						{
							x_dw[3] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
						strc = regstr[l];
						if (strc.find(x) < 50)
						{
							x_dw[3] = std::to_string(DW[l]);
							sx = sign_var[l];
							break;
						}
					}
					for (int k = 0; k < i; k++)
					{
						strc = instr[k];
						if (strc.find(y) < 50)
						{
							y_dw[3] = std::to_string(DW[k]);
							sy = sign_var[k];
							break;
						}
						strc = outstr[k];
						if (strc.find(y) < 50)
						{
							y_dw[3] = std::to_string(DW[k]);
							sy = sign_var[k];
							break;
						}
						strc = wirestr[k];
						if (strc.find(y) < 50)
						{
							y_dw[3] = std::to_string(DW[k]);
							sy = sign_var[k];
							break;
						}
						strc = regstr[k];
						if (strc.find(y) < 50)
						{
							y_dw[3] = std::to_string(DW[k]);
							sy = sign_var[k];
							break;
						}
					}
					count_DPC[2]++;							//count instances of SUB
					if ((sx == 0) && (sy == 0) && (sz == 0))
						oline = oline + "SUB #(.DATAWIDTH(" + z_dw[3] + ")) SUB_" + std::to_string(count_DPC[2]) + "(.a(" + x + "), .b(" + y + "), .diff(" + z + ")); \n";
					else
					{
						if (sx == 0)
						{
							x = "$signed({1'b0," + x + "}";
						}
						if (sy == 0)
						{
							y = "$signed({1'b0," + y + "}";
						}
						if (sz == 0)
						{
							z = "$signed({1'b0," + z + "}";
						}
						oline = oline + "SSUB #(.DATAWIDTH(" + z_dw[3] + ")) SSUB_" + std::to_string(count_DPC[2]) + "(.a(" + x + "), .b(" + y + "), .diff(" + z + ")); \n";
					}

					op_list[sum_count_DPC].dp_ins[0] = x;
					op_list[sum_count_DPC].dp_ins[1] = y;
					op_list[sum_count_DPC].dp_outs[0] = z;
					op_list[sum_count_DPC].function = 2;
					op_list[sum_count_DPC].order = sum_count_DPC + 1;
					op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
					//op_list[sum_count_DPC].latency = std::stof(strv);
					op_list[sum_count_DPC].out_line = oline;
					op_list[sum_count_DPC].d_width = std::stoi(z_dw[3]);
					sum_count_DPC++;

				}
			}
			found3 = iline.find(" * ");					//select MUL
			if (found3 != string::npos)
			{
				str = iline;
				(x, y, z) = iovalues(str, x, y, z);
				for (int m = 0; m < i; m++)
				{
					str2 = instr[m];
					str3 = outstr[m];
					str4 = wirestr[m];
					str5 = regstr[m];
					s = varcheck(nx, ny, nz, x, y, z, str2, str3, str4, str5);
					//here = here + s;
					if ((nx > 0) && (ny > 0) && (nz > 0))
						break;
				}
				if ((nx == 0) || (ny == 0) || (nz == 0))
				{
					cout << endl
						<< " Missing Variable " << endl;
					break;
				}
				here = 0;
				nw = 0, nx = 0, ny = 0, nz = 0;
				for (int g = 0; g < i; g++)
				{
					strc = instr[g];
					if (strc.find(z) < 50)
					{
						z_dw[4] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = outstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[4] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = wirestr[g];
					if (strc.find(z) < 50)
					{
						z_dw[4] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = regstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[4] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
				}
				for (int l = 0; l < i; l++)
				{
					strc = instr[l];
					if (strc.find(x) < 50)
					{
						x_dw[4] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = outstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[4] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = wirestr[l];
					if (strc.find(x) < 50)
					{
						x_dw[4] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = regstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[4] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
				}
				for (int k = 0; k < i; k++)
				{
					strc = instr[k];
					if (strc.find(y) < 50)
					{
						y_dw[4] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = outstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[4] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = wirestr[k];
					if (strc.find(y) < 50)
					{
						y_dw[4] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = regstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[4] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
				}
				count_DPC[3]++;							//count instances of MUL
				if ((sx == 0) && (sy == 0) && (sz == 0))
					oline = oline + "MUL #(.DATAWIDTH(" + z_dw[4] + ")) MUL_" + std::to_string(count_DPC[3]) + "(.a(" + x + "), .b(" + y + "), .prod(" + z + ")); \n";
				else
				{
					if (sx == 0)
					{
						x = "$signed({1'b0," + x + "}";
					}
					if (sy == 0)
					{
						y = "$signed({1'b0," + y + "}";
					}
					if (sz == 0)
					{
						z = "$signed({1'b0," + z + "}";
					}
					oline = oline + "SMUL #(.DATAWIDTH(" + z_dw[4] + ")) SMUL_" + std::to_string(count_DPC[3]) + "(.a(" + x + "), .b(" + y + "), .prod(" + z + ")); \n";
				}

				op_list[sum_count_DPC].dp_ins[0] = x;
				op_list[sum_count_DPC].dp_ins[1] = y;
				op_list[sum_count_DPC].dp_outs[0] = z;

				op_list[sum_count_DPC].function = 3;
				op_list[sum_count_DPC].order = sum_count_DPC + 1;
				op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
				//op_list[sum_count_DPC].latency = std::stof(strv);
				op_list[sum_count_DPC].out_line = oline;

				op_list[sum_count_DPC].d_width = std::stoi(z_dw[4]);
				sum_count_DPC++;

				temp = 3;
			}
			found4 = iline.find(" / ");					//select DIV
			if (found4 != string::npos)
			{
				str = iline;
				(x, y, z) = iovalues(str, x, y, z);
				for (int m = 0; m < i; m++)
				{
					str2 = instr[m];
					str3 = outstr[m];
					str4 = wirestr[m];
					str5 = regstr[m];
					s = varcheck(nx, ny, nz, x, y, z, str2, str3, str4, str5);
					//here = here + s;
					if ((nx > 0) && (ny > 0) && (nz > 0))
						break;
				}
				if ((nx == 0) || (ny == 0) || (nz == 0))
				{
					cout << endl
						<< " Missing Variable " << endl;
					break;
				}
				here = 0;
				nw = 0, nx = 0, ny = 0, nz = 0;
				for (int g = 0; g < i; g++)
				{
					strc = instr[g];
					if (strc.find(z) < 50)
					{
						z_dw[5] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = outstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[5] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = wirestr[g];
					if (strc.find(z) < 50)
					{
						z_dw[5] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = regstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[5] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
				}
				for (int l = 0; l < i; l++)
				{
					strc = instr[l];
					if (strc.find(x) < 50)
					{
						x_dw[5] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = outstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[5] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = wirestr[l];
					if (strc.find(x) < 50)
					{
						x_dw[5] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = regstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[5] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
				}
				for (int k = 0; k < i; k++)
				{
					strc = instr[k];
					if (strc.find(y) < 50)
					{
						y_dw[5] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = outstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[5] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = wirestr[k];
					if (strc.find(y) < 50)
					{
						y_dw[5] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = regstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[5] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
				}
				count_DPC[8]++;							//count instances of DIV
				if ((sx == 0) && (sy == 0) && (sz == 0))
					oline = oline + "DIV #(.DATAWIDTH(" + z_dw[5] + ")) DIV_" + std::to_string(count_DPC[8]) + "(.a(" + x + "), .b(" + y + "), .quot(" + z + ")); \n";
				else
				{
					if (sx == 0)
					{
						x = "$signed({1'b0," + x + "}";
					}
					if (sy == 0)
					{
						y = "$signed({1'b0," + y + "}";
					}
					if (sz == 0)
					{
						z = "$signed({1'b0," + z + "}";
					}
					oline = oline + "SDIV #(.DATAWIDTH(" + z_dw[5] + ")) SDIV_" + std::to_string(count_DPC[8]) + "(.a(" + x + "), .b(" + y + "), .quot(" + z + ")); \n";
				}
				op_list[sum_count_DPC].dp_ins[0] = x;
				op_list[sum_count_DPC].dp_ins[1] = y;
				op_list[sum_count_DPC].dp_outs[0] = z;

				op_list[sum_count_DPC].function = 8;
				op_list[sum_count_DPC].order = sum_count_DPC + 1;
				op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
				//op_list[sum_count_DPC].latency = std::stod(strv);
				op_list[sum_count_DPC].out_line = oline;

				op_list[sum_count_DPC].d_width = std::stoi(z_dw[5]);
				sum_count_DPC++;

				temp = 4;
			}
			found5 = iline.find(" % ");					//select MOD
			if (found5 != string::npos)
			{
				str = iline;
				(x, y, z) = iovalues(str, x, y, z);
				for (int m = 0; m < i; m++)
				{
					str2 = instr[m];
					str3 = outstr[m];
					str4 = wirestr[m];
					str5 = regstr[m];
					s = varcheck(nx, ny, nz, x, y, z, str2, str3, str4, str5);
					//here = here + s;
					if ((nx > 0) && (ny > 0) && (nz > 0))
						break;
				}
				if ((nx == 0) || (ny == 0) || (nz == 0))
				{
					cout << endl
						<< " Missing Variable " << endl;
					break;
				}
				here = 0;
				nw = 0, nx = 0, ny = 0, nz = 0;
				for (int g = 0; g < i; g++)
				{
					strc = instr[g];
					if (strc.find(z) < 50)
					{
						z_dw[6] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = outstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[6] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = wirestr[g];
					if (strc.find(z) < 50)
					{
						z_dw[6] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = regstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[6] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
				}
				for (int l = 0; l < i; l++)
				{
					strc = instr[l];
					if (strc.find(x) < 50)
					{
						x_dw[6] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = outstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[6] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = wirestr[l];
					if (strc.find(x) < 50)
					{
						x_dw[6] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = regstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[6] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
				}
				for (int k = 0; k < i; k++)
				{
					strc = instr[k];
					if (strc.find(y) < 50)
					{
						y_dw[6] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = outstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[6] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = wirestr[k];
					if (strc.find(y) < 50)
					{
						y_dw[6] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = regstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[6] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
				}
				count_DPC[9]++;							//count instances of MOD
				if ((sx == 0) && (sy == 0) && (sz == 0))
					oline = oline + "MOD #(.DATAWIDTH(" + z_dw[6] + ")) MOD_" + std::to_string(count_DPC[9]) + "(.a(" + x + "), .b(" + y + "), .rem(" + z + ")); \n";
				else
				{
					if (sx == 0)
					{
						x = "$signed({1'b0," + x + "}";
					}
					if (sy == 0)
					{
						y = "$signed({1'b0," + y + "}";
					}
					if (sz == 0)
					{
						z = "$signed({1'b0," + z + "}";
					}
					oline = oline + "SMOD #(.DATAWIDTH(" + z_dw[6] + ")) SMOD_" + std::to_string(count_DPC[9]) + "(.a(" + x + "), .b(" + y + "), .rem(" + z + ")); \n";
				}

				op_list[sum_count_DPC].dp_ins[0] = x;
				op_list[sum_count_DPC].dp_ins[1] = y;
				op_list[sum_count_DPC].dp_outs[0] = z;

				op_list[sum_count_DPC].function = 9;
				op_list[sum_count_DPC].order = sum_count_DPC + 1;
				op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
				//op_list[sum_count_DPC].latency = std::stof(strv);
				op_list[sum_count_DPC].out_line = oline;
				op_list[sum_count_DPC].d_width = std::stoi(z_dw[6]);
				sum_count_DPC++;

				temp = 5;
			}
			found6 = iline.find(" << ");					//select shift left, SHL
			if (found6 != string::npos)
			{
				str = iline;
				(x, y, z) = iovaluesshift(str, x, y, z);
				for (int m = 0; m < i; m++)
				{
					str2 = instr[m];
					str3 = outstr[m];
					str4 = wirestr[m];
					str5 = regstr[m];
					s = varcheck(nx, ny, nz, x, y, z, str2, str3, str4, str5);
					//here = here + s;
					if ((nx > 0) && (ny > 0) && (nz > 0))
						break;
				}
				if ((nx == 0) || (ny == 0) || (nz == 0))
				{
					cout << endl
						<< " Missing Variable " << endl;
					break;
				}
				here = 0;
				nw = 0, nx = 0, ny = 0, nz = 0;
				for (int g = 0; g < i; g++)
				{
					strc = instr[g];
					if (strc.find(z) < 50)
					{
						z_dw[7] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = outstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[7] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = wirestr[g];
					if (strc.find(z) < 50)
					{
						z_dw[7] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = regstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[7] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
				}
				for (int l = 0; l < i; l++)
				{
					strc = instr[l];
					if (strc.find(x) < 50)
					{
						x_dw[7] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = outstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[7] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = wirestr[l];
					if (strc.find(x) < 50)
					{
						x_dw[7] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = regstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[7] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
				}
				for (int k = 0; k < i; k++)
				{
					strc = instr[k];
					if (strc.find(y) < 50)
					{
						y_dw[7] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = outstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[7] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = wirestr[k];
					if (strc.find(y) < 50)
					{
						y_dw[7] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = regstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[7] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
				}
				count_DPC[7]++;							//count instances of SHL
				if ((sx == 0) && (sy == 0) && (sz == 0))
					oline = oline + "SHL #(.DATAWIDTH(" + z_dw[7] + ")) SHL_" + std::to_string(count_DPC[7]) + "(.a(" + x + "), .sh_amt(" + y + "), .d(" + z + ")); \n";
				else
				{
					if (sx == 0)
					{
						x = "$signed({1'b0," + x + "}";
					}
					if (sy == 0)
					{
						y = "$signed({1'b0," + y + "}";
					}
					if (sz == 0)
					{
						z = "$signed({1'b0," + z + "}";
					}
					oline = oline + "SSHL #(.DATAWIDTH(" + z_dw[7] + ")) SSHL_" + std::to_string(count_DPC[7]) + "(.a(" + x + "), .sh_amt(" + y + "), .d(" + z + ")); \n";
				}

				op_list[sum_count_DPC].dp_ins[0] = x;
				op_list[sum_count_DPC].dp_ins[1] = y;
				op_list[sum_count_DPC].dp_outs[0] = z;

				op_list[sum_count_DPC].function = 7;
				op_list[sum_count_DPC].order = sum_count_DPC + 1;
				op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
				//op_list[sum_count_DPC].latency = std::stof(strv);
				op_list[sum_count_DPC].out_line = oline;

				op_list[sum_count_DPC].d_width = std::stoi(z_dw[7]);
				sum_count_DPC++;

				temp = 6;
			}
			found7 = iline.find(" >> ");					//select shift right, SHR
			if (found7 != string::npos)
			{
				str = iline;
				(x, y, z) = iovaluesshift(str, x, y, z);
				for (int m = 0; m < i; m++)
				{
					str2 = instr[m];
					str3 = outstr[m];
					str4 = wirestr[m];
					str5 = regstr[m];
					s = varcheck(nx, ny, nz, x, y, z, str2, str3, str4, str5);
					//here = here + s;
					if ((nx > 0) && (ny > 0) && (nz > 0))
						break;
				}
				if ((nx == 0) || (ny == 0) || (nz == 0))
				{
					cout << endl
						<< " Missing Variable " << endl;
					break;
				}
				here = 0;
				nw = 0, nx = 0, ny = 0, nz = 0;
				for (int g = 0; g < i; g++)
				{
					strc = instr[g];
					if (strc.find(z) < 50)
					{
						z_dw[8] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = outstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[8] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = wirestr[g];
					if (strc.find(z) < 50)
					{
						z_dw[8] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = regstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[8] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
				}
				for (int l = 0; l < i; l++)
				{
					strc = instr[l];
					if (strc.find(x) < 50)
					{
						x_dw[8] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = outstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[8] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = wirestr[l];
					if (strc.find(x) < 50)
					{
						x_dw[8] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = regstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[8] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
				}
				for (int k = 0; k < i; k++)
				{
					strc = instr[k];
					if (strc.find(y) < 50)
					{
						y_dw[8] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = outstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[8] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = wirestr[k];
					if (strc.find(y) < 50)
					{
						y_dw[8] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = regstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[8] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
				}
				count_DPC[6]++;							//count instances of SHR
				if ((sx == 0) && (sy == 0) && (sz == 0))
					oline = oline + "SHR #(.DATAWIDTH(" + z_dw[8] + ")) SHR_" + std::to_string(count_DPC[6]) + "(.a(" + x + "), .sh_amt(" + y + "), .d(" + z + ")); \n";
				else
				{
					if (sx == 0)
					{
						x = "$signed({1'b0," + x + "}";
					}
					if (sy == 0)
					{
						y = "$signed({1'b0," + y + "}";
					}
					if (sz == 0)
					{
						z = "$signed({1'b0," + z + "}";
					}
					oline = oline + "SSHR #(.DATAWIDTH(" + z_dw[8] + ")) SSHR_" + std::to_string(count_DPC[6]) + "(.a(" + x + "), .sh_amt(" + y + "), .d(" + z + ")); \n";
				}

				op_list[sum_count_DPC].dp_ins[0] = x;
				op_list[sum_count_DPC].dp_ins[1] = y;
				op_list[sum_count_DPC].dp_outs[0] = z;

				op_list[sum_count_DPC].function = 6;
				op_list[sum_count_DPC].order = sum_count_DPC + 1;
				op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
				//op_list[sum_count_DPC].latency = std::stof(strv);
				op_list[sum_count_DPC].out_line = oline;

				op_list[sum_count_DPC].d_width = std::stoi(z_dw[8]);
				sum_count_DPC++;

				temp = 7;
			}
			found8 = iline.find(" ? ");					//select MUX
			if (found8 != string::npos)
			{
				str = iline;
				(w, x, y, z) = iovaluesmux(str, w, x, y, z);
				for (int m = 0; m < i; m++)
				{
					str2 = instr[m];
					str3 = outstr[m];
					str4 = wirestr[m];
					str5 = regstr[m];
					s = varcheck2(nw, nx, ny, nz, w, x, y, z, str2, str3, str4, str5);
					//here = here + s;
					if ((nw > 0) && (nx > 0) && (ny > 0) && (nz > 0))
						break;
				}
				if ((nw == 0) || (nx == 0) || (ny == 0) || (nz == 0))
				{
					cout << endl
						<< " Missing Variable " << endl;
					break;
				}
				here = 0;
				nw = 0, nx = 0, ny = 0, nz = 0;
				for (int g = 0; g < i; g++)
				{
					strc = instr[g];
					if (strc.find(z) < 50)
					{
						z_dw[9] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = outstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[9] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = wirestr[g];
					if (strc.find(z) < 50)
					{
						z_dw[9] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = regstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[9] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
				}
				for (int l = 0; l < i; l++)
				{
					strc = instr[l];
					if (strc.find(x) < 50)
					{
						x_dw[9] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}strc = outstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[9] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = wirestr[l];
					if (strc.find(x) < 50)
					{
						x_dw[9] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = regstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[9] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
				}
				for (int k = 0; k < i; k++)
				{
					strc = instr[k];
					if (strc.find(y) < 50)
					{
						y_dw[9] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}strc = outstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[9] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = wirestr[k];
					if (strc.find(y) < 50)
					{
						y_dw[9] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = regstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[9] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
				}
				for (int s = 0; s < i; s++)
				{
					strc = instr[s];
					if (strc.find(w) < 50)
					{
						w_dw[9] = std::to_string(DW[s]);
						sw = sign_var[s];
						break;
					}
					strc = outstr[s];
					if (strc.find(w) < 50)
					{
						w_dw[9] = std::to_string(DW[s]);
						sw = sign_var[s];
						break;
					}
					strc = wirestr[s];
					if (strc.find(w) < 50)
					{
						w_dw[9] = std::to_string(DW[s]);
						sw = sign_var[s];
						break;
					}
					strc = regstr[s];
					if (strc.find(w) < 50)
					{
						w_dw[9] = std::to_string(DW[s]);
						sw = sign_var[s];
						break;
					}
				}
				count_DPC[5]++;							//count instances of MUX
				if ((sx == 0) && (sy == 0) && (sz == 0) && (sw == 0))
					oline = oline + "MUX2x1 #(.DATAWIDTH(" + z_dw[9] + ")) MUX2x1_" + std::to_string(count_DPC[5]) + "(.a(" + x + "), .b(" + y + "), .sel(" + w + "), .d(" + z + ")); \n";
				else
				{
					if (sx == 0)
					{
						x = "$signed({1'b0," + x + "}";
					}
					if (sy == 0)
					{
						y = "$signed({1'b0," + y + "}";
					}
					if (sz == 0)
					{
						z = "$signed({1'b0," + z + "}";
					}
					if (sw == 0)
					{
						w = "$signed({1'b0," + w + "}";
					}
					oline = oline + "SMUX2x1 #(.DATAWIDTH(" + z_dw[9] + ")) SMUX2x1_" + std::to_string(count_DPC[5]) + "(.a(" + x + "), .b(" + y + "), .sel(" + w + "), .d(" + z + ")); \n";
				}

				op_list[sum_count_DPC].dp_ins[0] = x;
				op_list[sum_count_DPC].dp_ins[1] = y;
				op_list[sum_count_DPC].dp_ins[2] = w;
				op_list[sum_count_DPC].dp_outs[0] = z;
				op_list[sum_count_DPC].function = 5;
				op_list[sum_count_DPC].order = sum_count_DPC + 1;
				op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
				//op_list[sum_count_DPC].latency = std::stof(strv);
				op_list[sum_count_DPC].out_line = oline;

				op_list[sum_count_DPC].d_width = std::stoi(z_dw[9]);
				sum_count_DPC++;
				temp = 8;
			}
			found9 = iline.find(" == ");					//select COMP, eq output
			if (found9 != string::npos)
			{
				str = iline;
				(x, y, z) = iovaluescomp(str, x, y, z);
				for (int m = 0; m < i; m++)
				{
					str2 = instr[m];
					str3 = outstr[m];
					str4 = wirestr[m];
					str5 = regstr[m];
					s = varcheck(nx, ny, nz, x, y, z, str2, str3, str4, str5);
					//here = here + s;
					if ((nx > 0) && (ny > 0) && (nz > 0))
						break;
				}
				if ((nx == 0) || (ny == 0) || (nz == 0))
				{
					cout << endl
						<< " Missing Variable " << endl;
					break;
				}
				here = 0;
				nw = 0, nx = 0, ny = 0, nz = 0;
				for (int g = 0; g < i; g++)
				{
					strc = instr[g];
					if (strc.find(z) < 50)
					{
						z_dw[10] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = outstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[10] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = wirestr[g];
					if (strc.find(z) < 50)
					{
						z_dw[10] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = regstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[10] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
				}
				for (int l = 0; l < i; l++)
				{
					strc = instr[l];
					if (strc.find(x) < 50)
					{
						x_dw[10] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}strc = outstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[10] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = wirestr[l];
					if (strc.find(x) < 50)
					{
						x_dw[10] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = regstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[10] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
				}
				for (int k = 0; k < i; k++)
				{
					strc = instr[k];
					if (strc.find(y) < 50)
					{
						y_dw[10] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}strc = outstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[10] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = wirestr[k];
					if (strc.find(y) < 50)
					{
						y_dw[10] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = regstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[10] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
				}
				if (stoi(x_dw[10]) >= stoi(y_dw[10]))
					z_dw[10] = x_dw[10];
				else
					z_dw[10] = x_dw[10];
				count_DPC[4]++;							//count instances of COMP
				if (stoi(x_dw[10]) >= stoi(y_dw[10]))
					z_dw[10] = x_dw[10];
				else
					z_dw[10] = x_dw[10];

				op_list[sum_count_DPC].dp_ins[0] = x;
				op_list[sum_count_DPC].dp_ins[1] = y;
				op_list[sum_count_DPC].dp_outs[0] = z;

				op_list[sum_count_DPC].function = 4;
				op_list[sum_count_DPC].order = sum_count_DPC + 1;
				op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
				//op_list[sum_count_DPC].latency = std::stof(strv);
				op_list[sum_count_DPC].out_line = oline;
				op_list[sum_count_DPC].d_width = std::stoi(z_dw[10]);
				sum_count_DPC++;

				temp = 9;
			}
			found10 = iline.find(" < ");					//select COMP, lt output 
			if ((found10 != string::npos) && (temp != 6))
			{
				str = iline;
				(x, y, z) = iovaluescomp(str, x, y, z);
				for (int m = 0; m < i; m++)
				{
					str2 = instr[m];
					str3 = outstr[m];
					str4 = wirestr[m];
					str5 = regstr[m];
					s = varcheck(nx, ny, nz, x, y, z, str2, str3, str4, str5);
					//here = here + s;
					if ((nx > 0) && (ny > 0) && (nz > 0))
						break;
				}
				if ((nx == 0) || (ny == 0) || (nz == 0))
				{
					cout << endl
						<< " Missing Variable " << endl;
					break;
				}
				here = 0;
				nw = 0, nx = 0, ny = 0, nz = 0;
				for (int g = 0; g < i; g++)
				{
					strc = instr[g];
					if (strc.find(z) < 50)
					{
						z_dw[11] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = outstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[11] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = wirestr[g];
					if (strc.find(z) < 50)
					{
						z_dw[11] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = regstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[11] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
				}
				for (int l = 0; l < i; l++)
				{
					strc = instr[l];
					if (strc.find(x) < 50)
					{
						x_dw[11] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = outstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[11] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = wirestr[l];
					if (strc.find(x) < 50)
					{
						x_dw[11] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = regstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[11] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
				}
				for (int k = 0; k < i; k++)
				{
					strc = instr[k];
					if (strc.find(y) < 50)
					{
						y_dw[11] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = outstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[11] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = wirestr[k];
					if (strc.find(y) < 50)
					{
						y_dw[11] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = regstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[11] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
				}
				if (stoi(x_dw[11]) >= stoi(y_dw[11]))
					z_dw[11] = x_dw[11];
				else
					z_dw[11] = x_dw[11];
				count_DPC[4]++;							//count instances of COMP
				if ((sx == 0) && (sy == 0) && (sz == 0))
					oline = oline + "COMP #(.DATAWIDTH(" + z_dw[11] + ")) COMP_" + std::to_string(count_DPC[4]) + "(.a(" + x + "), .b(" + y + "), .lt(" + z + ")); \n";
				else
				{
					if (sx == 0)
					{
						x = "$signed({1'b0," + x + "}";
					}
					if (sy == 0)
					{
						y = "$signed({1'b0," + y + "}";
					}
					if (sz == 0)
					{
						z = "$signed({1'b0," + z + "}";
					}
					oline = oline + "SCOMP #(.DATAWIDTH(" + z_dw[11] + ")) SCOMP_" + std::to_string(count_DPC[4]) + "(.a(" + x + "), .b(" + y + "), .lt(" + z + ")); \n";
				}

				op_list[sum_count_DPC].dp_ins[0] = x;
				op_list[sum_count_DPC].dp_ins[1] = y;
				op_list[sum_count_DPC].dp_outs[0] = z;
				op_list[sum_count_DPC].function = 4;
				op_list[sum_count_DPC].order = sum_count_DPC + 1;
				op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
				//op_list[sum_count_DPC].latency = std::stof(strv);
				op_list[sum_count_DPC].out_line = oline;
				op_list[sum_count_DPC].d_width = std::stoi(z_dw[11]);
				sum_count_DPC++;

				temp = 10;
			}
			found11 = iline.find(" > ");					//select COMP, gt output
			if ((found11 != string::npos) && (temp != 7))
			{
				str = iline;
				(x, y, z) = iovaluescomp(str, x, y, z);
				for (int m = 0; m < i; m++)
				{
					str2 = instr[m];
					str3 = outstr[m];
					str4 = wirestr[m];
					str5 = regstr[m];
					s = varcheck(nx, ny, nz, x, y, z, str2, str3, str4, str5);
					//here = here + s;
					if ((nx > 0) && (ny > 0) && (nz > 0))
						break;
				}
				if ((nx == 0) || (ny == 0) || (nz == 0))
				{
					cout << endl
						<< " Missing Variable " << endl;
					break;
				}
				here = 0;
				nw = 0, nx = 0, ny = 0, nz = 0;
				for (int g = 0; g < i; g++)
				{
					strc = instr[g];
					if (strc.find(z) < 50)
					{
						z_dw[12] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = outstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[12] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = wirestr[g];
					if (strc.find(z) < 50)
					{
						z_dw[12] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = regstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[12] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
				}
				for (int l = 0; l < i; l++)
				{
					strc = instr[l];
					if (strc.find(x) < 50)
					{
						x_dw[12] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = outstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[12] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = wirestr[l];
					if (strc.find(x) < 50)
					{
						x_dw[12] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = regstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[12] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
				}
				for (int k = 0; k < i; k++)
				{
					strc = instr[k];
					if (strc.find(y) < 50)
					{
						y_dw[12] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = outstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[12] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = wirestr[k];
					if (strc.find(y) < 50)
					{
						y_dw[12] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
					strc = regstr[k];
					if (strc.find(y) < 50)
					{
						y_dw[12] = std::to_string(DW[k]);
						sy = sign_var[k];
						break;
					}
				}
				if (stoi(x_dw[12]) >= stoi(y_dw[12]))
					z_dw[12] = x_dw[12];
				else
					z_dw[12] = x_dw[12];
				count_DPC[4]++;							//count instances of COMP
				if ((sx == 0) && (sy == 0) && (sz == 0))
					oline = oline + "COMP #(.DATAWIDTH(" + z_dw[12] + ")) COMP_" + std::to_string(count_DPC[4]) + "(.a(" + x + "), .b(" + y + "), .gt(" + z + ")); \n";
				else
				{
					if (sx == 0)
					{
						x = "$signed({1'b0," + x + "}";
					}
					if (sy == 0)
					{
						y = "$signed({1'b0," + y + "}";
					}
					if (sz == 0)
					{
						z = "$signed({1'b0," + z + "}";
					}
					oline = oline + "SCOMP #(.DATAWIDTH(" + z_dw[12] + ")) SCOMP_" + std::to_string(count_DPC[4]) + "(.a(" + x + "), .b(" + y + "), .gt(" + z + ")); \n";
				}

				op_list[sum_count_DPC].dp_ins[0] = x;
				op_list[sum_count_DPC].dp_ins[1] = y;
				op_list[sum_count_DPC].dp_outs[0] = z;
				op_list[sum_count_DPC].function = 4;
				op_list[sum_count_DPC].order = sum_count_DPC + 1;
				op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
				//op_list[sum_count_DPC].latency = std::stof(strv);
				op_list[sum_count_DPC].out_line = oline;

				op_list[sum_count_DPC].d_width = std::stoi(z_dw[12]);
				sum_count_DPC++;
				temp = 11;
			}
			if ((found != string::npos) && (temp == 0))		//select REG
			{
				str = iline;
				(x, z) = iovaluesreg(str, x, z);
				for (int m = 0; m < i; m++)
				{
					str2 = instr[m];
					str3 = outstr[m];
					str4 = wirestr[m];
					str5 = regstr[m];
					s = varcheck3(nx, nz, x, z, str2, str3, str4, str5);
					//here = here + s;
					if ((nx > 0) && (nz > 0))
						break;
				}
				if ((nx == 0) || (nz == 0))
				{
					cout << endl
						<< " Missing Variable " << endl;
					break;
				}
				here = 0;
				nw = 0, nx = 0, ny = 0, nz = 0;
				for (int g = 0; g < i; g++)
				{
					strc = instr[g];
					if (strc.find(z) < 50)
					{
						z_dw[13] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = outstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[13] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = wirestr[g];
					if (strc.find(z) < 50)
					{
						z_dw[13] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
					strc = regstr[g];
					if (strc.find(z) < 50)
					{
						z_dw[13] = std::to_string(DW[g]);
						sz = sign_var[g];
						break;
					}
				}
				for (int l = 0; l < i; l++)
				{
					strc = instr[l];
					if (strc.find(x) < 50)
					{
						x_dw[13] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = outstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[13] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = wirestr[l];
					if (strc.find(x) < 50)
					{
						x_dw[13] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
					strc = regstr[l];
					if (strc.find(x) < 50)
					{
						x_dw[13] = std::to_string(DW[l]);
						sx = sign_var[l];
						break;
					}
				}

				error = opcheck(newline, x, z);
				if (error == 1)
				{
					cout << endl
						<< " Invalid Operation" << endl;
					break;
				}
				else
				{
					count_DPC[0]++;							//count instances of REG
					if ((sx == 0) && (sz == 0))
						oline = oline + "REG #(.DATAWIDTH(" + z_dw[13] + ")) REG_" + std::to_string(count_DPC[0]) + "(.d(" + x + "), .Clk(1), .Rst(0), .q(" + z + ")); \n";
					else
					{
						if (sx == 0)
						{
							x = "$signed({1'b0," + x + "}";
						}
						if (sz == 0)
						{
							z = "$signed({1'b0," + z + "}";
						}
						oline = oline + "SREG #(.DATAWIDTH(" + z_dw[13] + ")) SREG_" + std::to_string(count_DPC[0]) + "(.d(" + x + "), .Clk(1), .Rst(0), .q(" + z + ")); \n";
					}
					op_list[sum_count_DPC].dp_ins[0] = x;
					op_list[sum_count_DPC].dp_outs[0] = z;
					op_list[sum_count_DPC].function = 0;
					op_list[sum_count_DPC].order = sum_count_DPC + 1;
					op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
					//op_list[sum_count_DPC].latency = std::stof(strv);
					op_list[sum_count_DPC].out_line = oline;

					op_list[sum_count_DPC].d_width = std::stoi(z_dw[13]);
					sum_count_DPC++;
				}
			}
			count_dpc++;
		}
	}
	myfile2 << oline << '\n';
	myfile2 << "endmodule" << '\n';
	//seq_get_sequence();							//create schedule, calculates circuit_clocks
	//get_est_lat();							//populate .latentcy in structure
	//cr_dp = calc_cr_dp();					//calculate the critical data path
	myfile2 << endl << endl;
	myfile2 << "//Critical Path : " + std::to_string(cr_dp) << endl;		//print critical data path to output file

	myfile3.close();
	myfile2.close();

	return 0;
}

