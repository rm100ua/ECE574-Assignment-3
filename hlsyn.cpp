// hlsyn.cpp : Defines the entry point for the application.
//

#include "hlsyn.h"

//global variables

string ins_list = string();		//comma delimited list of inputs to the circuit
int circuit_clocks = 0;			//number of clock periods required to schedule this circuit

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

int main(int argc, char *argv[]) 
{
	string filename, filename1, filename2, iline, oline, newline, str, str1, strc, strv;
	string str2, str3, str4, str5, stri, stro;
	string instr[20] = {}, outstr[20] = {}, wirestr[20] = {}, regstr[20] = {}, varstr[20] = {};
	string insize, outsize, w, x, y, z, w_dw[14] = {}, x_dw[14] = {}, y_dw[14] = {}, z_dw[14] = {};
	string node[20] = {};
	size_t found, found1, found2, found3, found4, found5, found6;
	size_t found7, found8, found9, found10, found11, foundname1, foundname2;
	int bittemp, temp = 0, bitsize = 0, start = 0, i = 0, m = 0, DW[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	//array to count instances of each datapath component
	int s = 0, error = 0, here = 0, count_DPC[15] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 }; 
	int nw = 0, nx = 0, ny = 0, nz = 0, sw = 0, sx = 0, sy = 0, sz = 0;
	int ew, ex, ey, ez;
	int u = 0, sign_var[20] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };
	int sum_count_DPC = 0;		//count the datapath components, which is sum of count_DPC[]
	int c_period_req = 0;		//number of clock periods required by schedule
	float cr_dp = 0.0;			//critical data path

	//initailize the dpc_list array
	for (i = 0; i < 20; i++) {
		dpc_list[i].function = 99;		//component enumeration uses 0 thru 12
		dpc_list[i].order = 0;
		//dpc_list[i].top_order = 0;
		dpc_list[i].out_line = string();
		//dpc_list[i].d_width = 0;
		//dpc_list[i].latency = 0.0;
		//dpc_list[i].i_clock = 0;
		dpc_list[i].dp_ins[0] = string();
		dpc_list[i].dp_ins[1] = string();
		dpc_list[i].dp_ins_str = string();
		dpc_list[i].dp_outs[0] = string();
		dpc_list[i].dp_outs[1] = string();
		dpc_list[i].dp_outs[2] = string();
		dpc_list[i].dp_outs_str = string();
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
