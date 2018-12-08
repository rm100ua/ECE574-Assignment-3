`timescale 1ns / 1ns 

module HLSM (Clk, Rst, Start, a, b, c, start, end, z, x, Done); 
input Clk, Rst, Start; 
output reg Done; 
input signed [31:0] a, b, c, start, end; 
output reg signed [31:0] z, x; 
reg signed [31:0] d, e, f, g ,h, state; 
reg signed [0:0] dLTe, dEQe, state; 
reg signed [7:0] i, state; 

always@(posedge Clk) begin 
if (Rst == 1'b1) begin 
	state <= 0; 
	z <= 0; 
	x <= 0; 
end else begin 
	case(state) 
		0 : if (Start == 1'b1) begin
			state <= 1;
			Done <= 0; 
			z <= 0; 
			x <= 0; 
		end else begin 
			state <= 0; 
		end
		1 : if (Start == 1'b1) begin
			state <= 2;
			d = a + b;
			e = a + c;
			f = a - b  ;
			end
		2 : if (Start == 1'b1) begin
			state <= 3;
			dEQe = d == e;
			dLTe = d < e;
			end
		3 : if (Start == 1'b1) begin
			state <= 4;

		4 : if (Start == 1'b1) begin
			state <= 5;

		5 : if (Start == 1'b1) begin
			state <= 6;

		6 : if (Start == 1'b1) begin
			state <= 7;

		7 : if (Start == 1'b1) begin
			state <= 8;

		8 : if (Start == 1'b1) begin
			state <= 9;
			for ( i = start; i < end; i = i + 1 ) {;
			end
		9 : if (Start == 1'b1) begin
			state <= 10;
			g = dLTe ? d : e;
			h = dEQe ? g : f;
			x = h << dLTe;
			end
		10 : if (Start == 1'b1) begin
			state <= 11;
			z = h >> dEQe;
			end
		11 : if (Start == 1'b1) begin
			state <= 0;
			Done <= 1; 
			end
		default : state <= 0; 
	endcase 
end
end 
endmodule 
