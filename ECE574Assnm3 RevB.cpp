// hlsyn.cpp : Defines the entry point for the application.
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
#include "hlsyn.h"

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
	string dp_ins[3];			//array of inputs
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
};								//Data Path Component (DPC) structure with DPC attributes
struct dp_comp op_list[20];	//create array of above structure


//global structure contains attributes of the Circuit

struct cir_desc {			//circuit description
	string inp_str;			//input string
	string ins[20];			//array of inputs to the circuit
	string inp_type;		//Int or UInt
	string out_str;
	string var_str;
	string outs[20];			//array of outputs of the circuit
	string wires[4];
	string reg[20];
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

void get_in_out_data(string str, string &str1, string &str2, string &str3, string &str4, int &bitsize) 
{
	size_t foundstr1, foundstr2, foundstr3;
	
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
		bitsize = stoi(str.substr(0, foundstr3));
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
		bitsize = stoi(str.substr(0, foundstr3));
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
		bitsize = stoi(str.substr(0, foundstr3));
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

//generic routines to find input and output variables for datapath components

void iovalues(string str, string &x, string &y, string &z) //find input and output variables for structure
{
	z = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	x = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	y = str.substr(0, str.find(" "));
	return;
}

//MUX  find the input and output variables for MUX

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


//REG  find the input and output variables for REG

void iovaluesreg(string str, string &x, string &z) //find input and output variables for structure
{
	z = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	x = str.substr(0, str.find(" "));
	return;
}

//SHIFT  find the input and output variables for SHR and SHL

void iovaluesshift(string str, string &x, string &y, string &z) //find input and output variables for structure
{
	z = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 3));
	x = str.substr(0, str.find(" "));
	str = str.substr((str.find(" ") + 4));
	y = str.substr(0, str.find(" "));
	return;
}

//COMP  find the input and output variables for COMP

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

/*void parse_cir_inputs(void)
{
	string str;
	size_t found;
	int c = 0, d = 0, f = 0;

	str = cir_list.inp_str;
	while (str != "")
	{
		found = str.find(",");
		if ((found != string::npos) && (found < str.length()))
		{
			cir_list.ins[c] = str.substr(0, found);
			str = str.substr(found + 1);
			c++;
		}
		if (found == string::npos)
		{
			cir_list.ins[c] = str;
			str = "";
		}			
	}
	str = cir_list.out_str;
	while (str != "")
	{
		found = str.find(",");
		if ((found != string::npos) && (found < str.length()))
		{
			cir_list.outs[d] = str.substr(0, found);
			str = str.substr(found + 1);
			d++;
		}
		if (found == string::npos)
		{
			cir_list.outs[d] = str;
			str = "";
		}
	}
	str = cir_list.var_str;
	while (str != "")
	{
		found = str.find(",");
		if ((found != string::npos) && (found < str.length()))
		{
			cir_list.reg[f] = str.substr(0, found);
			str = str.substr(found + 1);
			f++;
		}
		if (found == string::npos)
		{
			cir_list.reg[f] = str;
			str = "";
		}
	}
}*/



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
	for (p = 0; p < 3; ++p) 
	{
		cout << cir_list.ins[p] << endl;
	}
	cout << endl << endl;

	cout << "ready_inputs:  " << endl;

	for (p = 0; p < 3; ++p) 
	{
		ready_inputs[p] = cir_list.ins[p];		//copy circuit inpputs
		cout << ready_inputs[p] << ", ";
	}
	cout << endl;
	   
	for (p = 4; p < 15; p++)
	{
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

	for (p = 0; p < dpc_item_count; p++) 
	{
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

double calcTForce(int node, int t_period)
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

int main(int argc, char *argv[]) 
{
	string filename, filename1, filename2, iline, oline, newline, str, str1, strc, strv;
	string str2, str3, str4, str5, stri, stro, w, x, y, z;
	string instr[20] = {}, outstr[20] = {},/* wirestr[20] = {}, regstr[20] = {},*/ varstr[20] = {};
	//string w_dw[14] = {}, x_dw[14] = {}, y_dw[14] = {}, z_dw[14] = {};
	string node[20] = {}, insize, outsize;
	size_t found, found1, found2, found3, found4, found5, found6;
	size_t found7, found8, found9, found10, found11, foundname1, foundname2;
	int bittemp, temp = 0, bitsize = 0, start = 0, i = 0, m = 0, DW[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	//array to count instances of each datapath component
	int s = 0, error = 0, here = 0, count_DPC[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; 
	//int nw = 0, nx = 0, ny = 0, nz = 0, sw = 0, sx = 0, sy = 0, sz = 0;
	int ew, ex, ey, ez;
	int u = 0, sign_var[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	int sum_count_DPC = 0;		//count the datapath components, which is sum of count_DPC[]
	int c_period_req = 0;		//number of clock periods required by schedule
	float cr_dp = 0.0;			//critical data path

	//initialize the cir_list array

	for (i = 0; i < 4; i++) 
	{
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
		op_list[i].dp_ins[2] = string();
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

	filename2 = filename1.substr(0, filename1.find("."));
	filename2 = filename2 + ".v";

	ifstream myfile1(filename1); // open input file
	ofstream myfile2(filename2); //open output file

	if (myfile1.is_open()) // open input file check and write to output file check
	{
		s = 0;
		int count_dpc = 0;					//count the datapath components found
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
				temp = 16;
				node[s] = iline;	//add "if" statement to vertex/node array
				op_list[s].dp_ins_str = iline;
				op_list[s].order = s;
				op_list[s].function = temp;
				s++;		//count number of nodes/vectors
			}
			found = iline.find("else");
			if ((found != string::npos) && (found < iline.length()))
			{
				temp = 17;
				node[s] = iline;	//add "else" statement to vertex/node array
				op_list[s].dp_ins_str = iline;
				op_list[s].order = s;
				op_list[s].function = temp;
				s++;			//count number of nodes/vectors
			}
			if (iline != "")
			{
				found1 = iline.find("=");
				if (found1 != string::npos)
				{
					node[s] = iline;	//add to vertex/node array
					op_list[s].dp_ins_str = iline;
					op_list[s].order = s;
					//s++; 
					found = iline.find("for");
					temp = 18;
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
						w = ""; x = ""; y = ""; z = "";
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
							op_list[s].dp_ins[0] = w;
							op_list[s].dp_ins[1] = x;
							op_list[s].dp_ins[2] = y;
							op_list[s].dp_outs[0] = z;
							count_DPC[temp]++;
							//op_list[sum_count_DPC].function = 10;
							//op_list[sum_count_DPC].order = sum_count_DPC + 1;
							//op_list[sum_count_DPC].top_order = sum_count_DPC + 1;
							//op_list[sum_count_DPC].latency = std::stod(strv);
							//op_list[sum_count_DPC].out_line = oline;
							//op_list[sum_count_DPC].d_width = std::stoi(z_dw[0]);
							//sum_count_DPC++;
							error = ew + ex + ey + ez;
						}
						if (error != 0)
						{
							cout << endl
								<< " Missing Variable" << endl;
							return 2;
						}
					}
					op_list[s].function = temp;
					s++;			//count number of nodes/vectors
				}
				else
				{
					get_in_out_data(iline, str1, str2, str3, str4, bitsize);
					instr[i] = str1;
					/*if (str1 != "")
						if (cir_list.inp_str == "")
							cir_list.inp_str = cir_list.inp_str + str1;
						else
							cir_list.inp_str = cir_list.inp_str + ", " + str1;*/
					outstr[i] = str2;
					/*if (str2 != "")
						if (cir_list.out_str == "")
							cir_list.out_str = cir_list.out_str + str2;
						else
							cir_list.out_str = cir_list.out_str + ", " + str2;*/
					varstr[i] = str3;
					/*if (str3 != "")
						if (cir_list.var_str == "")
							cir_list.var_str = cir_list.var_str + str3;
						else
							cir_list.var_str = cir_list.var_str + ", " + str3;*/
					sign_var[i] = stoi(str4);		// 0 == unsigned ans 1 == signed
					DW[i] = bitsize;
					i++;			//count number of inputs/utput/variable lines
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

	//get_sequence();

	if (myfile2.is_open()) // open output file check and write to output file
	{
		oline = "`timescale 1ns / 1ns \n\n";
		oline = oline + "module HLSM (Clk, Rst, Start, Done, ";
		for (m = 0; m < i; m++)
		{
			if (instr[m] != "")
			{
				oline = oline + instr[m];
			}
			if (outstr[m] != "")
			{
				oline = oline + outstr[m];
			}
			if ((instr[m + 1] != "") || (outstr[m + 1] != ""))
				oline = oline + ", ";
		}
		oline = oline + ") \n";
		myfile2 << oline;
		myfile2 << "input Clk, Rst, Start; \n";
		myfile2 << "output reg Done; \n";

		for (u = 0; u < i; u++)
		{
			if (instr[u] != "")
			{
				bittemp = DW[u] - 1;
				if (sign_var[u] == 0)
					newline = "input [" + std::to_string(bittemp + 1) + ":0] " + instr[u] + " \n";
				else
					newline = "input signed [" + std::to_string(bittemp) + ":0] " + instr[u] + " \n";
				myfile2 << newline;
			}
			if (outstr[u] != "")
			{
				bittemp = DW[u] - 1;
				if (sign_var[u] == 0)
					newline = "output reg [" + std::to_string(bittemp + 1) + ":0] " + outstr[u] + " \n";
				else
					newline = "output reg signed [" + std::to_string(bittemp) + ":0] " + outstr[u] + " \n";
				myfile2 << newline;
			}
			if (varstr[u] != "")
			{
				bittemp = DW[u] - 1;
				if (sign_var[u] == 0)
					newline = "register [" + std::to_string(bittemp + 1) + ":0] " + varstr[u] + " \n";
				else 
					newline = "register signed [" + std::to_string(bittemp) + ":0] " + varstr[u] + " \n";
				myfile2 << newline;
			}
		}

		myfile2 << "\n";
		myfile2 << "endmodule \n";
	}
	else
	{
		cout << "Unable to open file";
		return 1;
	}
	myfile2.close();
	return 0;
}