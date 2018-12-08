`timescale 1ns / 1ns 

module HLSM (Clk, Rst, Start, a, b, c, zero, one, t, z, x, Done); 
input Clk, Rst, Start; 
output reg Done; 
input signed [31:0] a, b, c, zero, one; 
input signed [0:0] t; 
output reg signed [31:0] z, x; 
reg signed [31:0] e, g, d, f, state; 

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
			if ( t );
			d = a - one;
			x = f - d;
			g = d > e;
			end
		2 : if (Start == 1'b1) begin
			state <= 3;
			else {;
			z = g ? d : e;
			end
		3 : if (Start == 1'b1) begin
			state <= 4;
			e = a + c;
			end
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

		9 : if (Start == 1'b1) begin
			state <= 10;

		10 : if (Start == 1'b1) begin
			state <= 11;
			f = a * c;
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