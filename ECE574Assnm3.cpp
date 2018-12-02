// hlsyn.cpp : Defines the entry point for the application.
//

//#include "hlsyn.h"
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
int sum_count_DPC = 0;		//count the datapath components, which is sum of count_DPC[]

/***
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
	//poplulate by ASAP routine
	int ASAP_clock;				//clock cycle for ASAP schedule
	//populate by ALAP routine
	int ALAP_clock;				//clock cycle for ALAP schedule
	//populate by width routine
	int width;					//time-frame width
	string out_line;			//output line to be sent to output file (verilog file)
};								//Data Path Component (DPC) structure with DPC attributes
struct dp_comp dpc_list[20];	//create array of above structure
***/

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

void get_in_out_data(string str, string &str1, string &str2, string &str3, string &str4) 
{
	size_t foundstr1, foundstr2, foundstr3;
	string inputstr, outstr, varstr;

	foundstr1 = str.find("input");
	if (foundstr1 != string::npos)
	{
		if ((foundstr1 = str.find("UInt")) != string::npos) {
			str = str.substr(foundstr1 + 4);
			//cir_list.inp_type = "UInt";	
		}
		else if ((foundstr2 = str.find("Int")) != string::npos) {
			str = str.substr(foundstr2 + 3);
			//cir_list.inp_type = "Int";
			str4 = "1";
		}
		foundstr3 = str.find(" ");
		if (foundstr3 != string::npos)
		{
			str1 = str.substr(foundstr3 + 1);
		}
	}
	foundstr1 = str.find("output");
	if (foundstr1 != string::npos)
	{
		if ((foundstr1 = str.find("UInt")) != string::npos) {
			str = str.substr(foundstr1 + 4);
			//cir_list.inp_type = "UInt";	
		}
		else if ((foundstr2 = str.find("Int")) != string::npos) {
			str = str.substr(foundstr2 + 3);
			//cir_list.inp_type = "Int";
			str4 = "1";
		}
		foundstr3 = str.find(" ");
		if (foundstr3 != string::npos)
		{
			str2 = str.substr(foundstr3 + 1);
		}
	}
	foundstr1 = str.find("variable");
	if (foundstr1 != string::npos)
	{
		if ((foundstr1 = str.find("UInt")) != string::npos) {
			str = str.substr(foundstr1 + 4);
			//cir_list.inp_type = "UInt";	
		}
		else if ((foundstr2 = str.find("Int")) != string::npos) {
			str = str.substr(foundstr2 + 3);
			//cir_list.inp_type = "Int";
			str4 = "1";
		}
		foundstr3 = str.find(" ");
		if (foundstr3 != string::npos)
		{
			str3 = str.substr(foundstr3 + 1);
		}
	}
	return;
}

void get_ops_data(string str ,int &temp)
{
	size_t found, found1, found2, found3, found4, found5, found6, found7;
	size_t found8, found9, found10, found11;

	found = str.find(" = "); // detemine which operation is being performed
	if ((found != string::npos) && (found < 50))
	{
		str = str.substr(found + 2);
		found1 = str.find(" + ");					//select ADD or INC
		if (found1 != string::npos)
		{
			//temp = 1;
			found1 = str.find("+ 1");
			if (found1 != string::npos)
			{
				temp = 1;
				//count_DPC[10]++;							//count instances of INC
			}
			else
			{
				temp = 2;
				//count_DPC[1]++;								//count instances of ADD
			}
		}
		found2 = str.find(" - ");					//select SUB or DEC
		if (found2 != string::npos)
		{
			//temp = 2;
			found2 = str.find("- 1");
			if (found2 != string::npos)
			{
				temp = 3;
				//count_DPC[11]++;							//count instances of DEC
			}
			else
			{
				temp = 4;
				//count_DPC[2]++;							//count instances of SUB
			}
		}
		found3 = str.find(" * ");					//select MUL
		if (found3 != string::npos)
		{
			temp = 5;
			//count_DPC[3]++;							//count instances of MUL
		}
		found4 = str.find(" / ");					//select DIV
		if (found4 != string::npos)
		{
			temp = 6;
			//count_DPC[8]++;							//count instances of DIV
		}
		found5 = str.find(" % ");					//select MOD
		if (found5 != string::npos)
		{
			temp = 7;
			//count_DPC[9]++;							//count instances of MOD
		}
		found6 = str.find(" << ");					//select shift left, SHL
		if (found6 != string::npos)
		{
			temp = 8;
			//count_DPC[7]++;							//count instances of SHL
		}
		found7 = str.find(" >> ");					//select shift right, SHR
		if (found7 != string::npos)
		{
			temp = 9;
			//count_DPC[6]++;							//count instances of SHR
		}
		found8 = str.find(" ? ");					//select MUX
		if (found8 != string::npos)
		{
			temp = 10;
			//count_DPC[5]++;							//count instances of MUX
		}
		found9 = str.find(" == ");					//select COMP, eq output
		if (found9 != string::npos)
		{
			temp = 11;
			//count_DPC[4]++;							//count instances of COMP
		}
		found10 = str.find(" < ");					//select COMP, lt output 
		if ((found10 != string::npos) && (temp != 6))
		{
			temp = 12;
			//count_DPC[4]++;							//count instances of COMP
		}
		found11 = str.find(" > ");					//select COMP, gt output
		if ((found11 != string::npos) && (temp != 7))
		{
			temp = 13;
			//count_DPC[4]++;							//count instances of COMP
		}
		if ((found != string::npos) && (temp == 0))		//select REG
		{
			temp = 15;
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
	string ops[11] = { "+", "-", "*", "/", "%", "<", ">", "==", "<<", ">>", "?" };
	h = str.find(" ");
	str = str.substr(h + 1);
	h = str.find(" ");
	str = str.substr(h + 1);
	h = str.find(" ");
	str = str.substr(h + 1);
	op = str.substr(0, (str.find(" ")));
	for (int b = 0; b < 12; b++)
	{
		if ((op != ops[b]) && (op != " "))
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

void varcheck(string str, string str1, string str2, string str3, string w, string x, string y, string z, int &ew, int &ex, int &ey, int &ez) //operation check
{
	int e = 0;
	//int dw = 0, dx = 0, dy = 0, dz = 0;
	size_t found1, found2, found3;
	if (ex != 0)
	{
		found1 = str1.find(x);
		found2 = str2.find(x);
		found3 = str3.find(x);
		if ((found1 != string::npos) || (found2 != string::npos) || (found3 != string::npos))
			ex = 0;
		else
			ex = 2;
	}
	if ((y != "") && (ey != 0))
	{
		found1 = str1.find(y);
		found2 = str2.find(y);
		found3 = str3.find(y);
		if ((found1 != string::npos) || (found2 != string::npos) || (found3 != string::npos))
			ey = 0;
		else
			ey = 2;
	}
	else
		ey = 0;
	if (ez != 0)
	{
		found1 = str1.find(z);
		found2 = str2.find(z);
		found3 = str3.find(z);
		if ((found1 != string::npos) || (found2 != string::npos) || (found3 != string::npos))
			ez = 0;
		else
			ez = 2;
	}
	if ((w != "") && (ew != 0))
	{
		found1 = str1.find(w);
		found2 = str2.find(w);
		found3 = str3.find(w);
		if ((found1 != string::npos) || (found2 != string::npos) || (found3 != string::npos))
			ew = 0;
		else
			ew = 2;
	}
	else
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
	return;
}

/***
//routine to find non-schedule sequence
//populate the seq_clock value of the structure for each operation
//this is a CDFG, it gives order but no schedule
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
	//int dpc_item_count = 0;					//number of DPC in circuit
	int dpc_schd = 0;						//count dpc scheduled
	int rdy_idx = 0;
	int inp_found = 0;						//flag inputs as found in ready input list

	cout << "get_sequence" << endl;

	//cout << cir_list.inp_str << endl;
	//cout << "output string" << cir_list.out_str << endl;

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
	} while (dpc_schd < op_num);			//repeat until all components have been scheduled

	circuit_clocks = seq_clock;			//number of clock cycles required for this circuit
	for (p = 0; p < op_num; p++) {
			cout << "p i.clock:  " << p << "  " << op_list[p].seq_clock << endl;
	}
}
***/
//routine to find schedule, that is populate the .i_clock value of the structure for each DPC
void get_schedule(void)
{
	int p = 0;								//counter used in for loop
	int n = 0;
	int k = 0;
	int inp_idx;								//index for inputs arrays
	int dpc_idx;							//index for data path components arrays
	int i_clock = 1;						//clock periods used for scheduling
	int count_dpc_scheduled = 0;			//count the Data Path Components (DPC) scheduled
	string ready_inputs[15];				//array of inputs that are ready, allowing datapaths to be scheduled
	string dpc_inputs[12][10];				//DPC inputs [dpc_list.order][inp_index]
	string dpc_outputs[12][10];				//DPC inputs [dpc_list.order][inp_index]
	string temp_str = string();				//temporary string
	int beg_pos = 0;						//string index
	int end_pos = 0;
	int count_dpc = 0;						//count the components
	int ready_ins_count;					//number of inputs in ready_inputs list
	//int dpc_item_count = 0;					//number of DPC in circuit
	int dpc_schd = 0;						//count dpc scheduled
	int rdy_idx = 0;
	int inp_found = 0;						//flag inputs as found in ready input list
	int circuit_ins_count = 0;

	cout << "get_schedule" << endl;
	//parse_cir_inputs();

	cout << cir_list.inp_str << endl;
	cout << "output string" << cir_list.out_str << endl;

	cout << "circuit inputs:  " << endl;
	for (p = 0; p < 3; p++) {
		cout << cir_list.ins[p] << endl;
	}
	cout << endl << endl;

	do {
		circuit_ins_count++;				//count the number of circuit inpuuts
	} while (cir_list.ins[circuit_ins_count] != "\0");

	cout << "ready_inputs:  " << endl;
	for (p = 0; p < circuit_ins_count; p++) {
		ready_inputs[p] = cir_list.ins[p];		//copy circuit inpputs
		cout << ready_inputs[p] << ", ";
	}
	cout << endl;

	for (p = 4; p < circuit_ins_count; p++) {
		ready_inputs[p] = string();		//empty string
		//inp_found[p] = 0;				//set input found array elements to zero
	}

	ready_ins_count = 0;
	do {
		ready_ins_count++;				//count the number of ready inpuuts
	//} while (!ready_inputs[ready_ins_count].empty());
	} while (ready_inputs[ready_ins_count] != "\0");
	cout << "ready inputs:  " << ready_ins_count << endl;

	//DPC that can run in first clock period have inputs in the circuit inputs list
	dpc_idx = 0;
	
	while (op_list[dpc_idx].order) {			//check every dpc against the circuit inputs
		if ((op_list[dpc_idx].dp_ins[0] == cir_list.ins[0]) || (op_list[dpc_idx].dp_ins[0] == cir_list.ins[1]) || (op_list[dpc_idx].dp_ins[0] == cir_list.ins[2])
			&& (op_list[dpc_idx].dp_ins[1] == cir_list.ins[0]) || (op_list[dpc_idx].dp_ins[1] == cir_list.ins[1]) || (op_list[dpc_idx].dp_ins[1] == cir_list.ins[2]))
		{
			op_list[dpc_idx].i_clock = i_clock;			//schedule the dpc
			cout << ".i_clock  " << dpc_idx << ": " << op_list[p].i_clock << endl;

			p = 0;											//add datapath outputs to ready_ouputs array
			do {											//all dpc have at least 1 input
				//ready_inputs[ready_ins_count - 1] = cir_list.outs[p];
				ready_inputs[ready_ins_count++] = op_list[dpc_idx].dp_outs[p];
				p++;
				//} while( ! cir_list.ins[p].empty() );
			//} while (!cir_list.ins[p].empty());
			} while ((!op_list[dpc_idx].dp_outs[p].empty()) && (p < 3));
			dpc_schd++;										//count dpc scheduled
		}
		dpc_idx++;
	}

	//check every DPC against the ready inputs list; then increment the clock
	do {						//clock loop
		i_clock++;				//advance to next clock period
		dpc_idx = 0;
		do {										//dpc loop
			if (!op_list[dpc_idx].i_clock)			//if this DPC is NOT scheduled, compare inputs to ready_inputs
			{
				inp_idx = 0;
				do {								//dpc inputs loop
					//inp_idx++;			//advance to next input
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
				if (inp_found && !op_list[dpc_idx].dp_ins[inp_idx - 1].empty())
				{
					op_list[dpc_idx].i_clock = i_clock;			//schedule the DPC for this clock period
					p = 0;
					do {											//all dpc have at least 1 input
						ready_inputs[ready_ins_count++] = op_list[dpc_idx].dp_outs[p];
						p++;
					} while ((!op_list[dpc_idx].dp_outs[p].empty()) && (p < 3));
					dpc_schd++;										//count dpc scheduled
				}
			}  //if (!op_list[dpc_idx].i_clock)
			dpc_idx++;			//advance to next dpc
		} while (op_list[dpc_idx].order);						//.order is zero if array is empty	
	} while ((dpc_schd < sum_count_DPC) && (i_clock < 222));			//repeat until all components have been scheduled

	circuit_clocks = i_clock;			//number of clock cycles required for this circuit
	for (p = 0; p < sum_count_DPC; p++) {
		cout << "p i.clock:  " << p << "  " << op_list[p].i_clock << endl;
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

	do {
		switch (op_list[p].function)		// (ALU = 0, MUL = 1, Logic / Logical = 2, Div = 3)
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
	int ps_node1 = 0;			//predecessor or successor node
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

int main(int argc, char *argv[]) 
{
	string filename, filename1, filename2, iline, oline, newline, str, str1, strc, strv;
	string str2, str3, str4, str5, stri, stro;
	string instr[20] = {}, outstr[20] = {}, wirestr[20] = {}, regstr[20] = {}, varstr[20] = {};
	string insize, outsize, w, x, y, z, w_dw[14] = {}, x_dw[14] = {}, y_dw[14] = {}, z_dw[14] = {};
	string node[20] = {};
	size_t found;
	//found1, found2, found3, found4, found5, found6;
	//size_t found7, found8, found9, found10, found11, 
	size_t foundname1;		// , foundname2;
	//int bittemp, 
	int temp = 0, bitsize = 0, start = 0, i = 0, m = 0, DW[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	//array to count instances of each datapath component
	int s = 0, error = 0, here = 0, count_DPC[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; 
	int nw = 0, nx = 0, ny = 0, nz = 0, sw = 0, sx = 0, sy = 0, sz = 0;
	int ew, ex, ey, ez;
	int u = 0, sign_var[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	int sum_count_DPC = 0;		//count the datapath components, which is sum of count_DPC[]
	int c_period_req = 0;		//number of clock periods required by schedule
	float cr_dp = 0.0;			//critical data path

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
	cin >> filename1;
	ifstream myfile1(filename1); // open input file
	//ofstream myfile2(filename2); //open output file

	if (myfile1.is_open()) // open input file check and write to output file check
	{
		s = 0;
		while (getline(myfile1, iline)) //parse the input variables
		{
			while (iline.find(" ") == 0)
				iline = iline.substr((iline.find(" ")) + 1);	//remove extra preceding spaces
			while (iline.find("\t") == 0)
				iline = iline.substr((iline.find("\t")) + 1);	//remove preceding tab characters
			str1 = ""; str2 = ""; str3 = ""; str4 = "0";
			temp = 0; 
			found = iline.find("if");
			if ((found != string::npos) && (found < iline.length()))
			{
				node[s] = iline;	//add "if" statement to vertex/node array
				s++;
			}
			found = iline.find("else");
			if ((found != string::npos) && (found < iline.length()))
			{
				node[s] = iline;	//add "else" statement to vertex/node array
				s++;
			}
			if (iline != "")
			{
				foundname1 = iline.find("=");
				if (foundname1 != string::npos)
				{
					node[s] = iline;	//add to vertex/node array
					s++; 
					found = iline.find("for");
					if (found == string::npos)
					{
						get_ops_data(iline, temp);
						count_DPC[temp]++;
						error = opcheck(iline);
						if (error == 1)
						{
							cout << endl
								<< " Invalid Operation" << endl;
							return 2;
						}
						ew = 1; ex = 1; ey = 1; ez = 1;
						for (m = 0; m < i; m++)
						{
							stri = instr[m];
							stro = outstr[m];
							strv = varstr[m];
							switch (temp)
							{
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
							error = ew + ex + ey + ez;
						}
						if (error != 0)
						{
							cout << endl
								<< " Missing Variable" << endl;
							return 2;
						}
					}
				}
				else
				{
					get_in_out_data(iline, str1, str2, str3, str4);
					instr[i] = str1;
					outstr[i] = str2;
					varstr[i] = str3;
					sign_var[i] = stoi(str4);
					i++;
				}				
			}			
		}
	}
	else
	{
		cout << "Unable to open file";
		return 1;
	}	
	
	
	myfile1.close();
	
	
	return 0;
}
