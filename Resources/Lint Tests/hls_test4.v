`timescale 1ns / 1ns 

module HLSM (Clk, Rst, Start, a, b, c, d, zero, z, Done); 
input Clk, Rst, Start; 
output reg Done; 
input [64:0] a, b, c, d, zero; 
output reg [64:0] z; 
reg [64:0] e, f, g, state; 
reg [1:0] gEQz  , state; 

always@(posedge Clk) begin 
if (Rst == 1'b1) begin 
	state <= 0; 
	z <= 0; 
end else begin 
	case(state) 
		0 : if (Start == 1'b1) begin
			state <= 1;
			Done <= 0; 
			z <= 0; 
		end else begin 
			state <= 0; 
		end
		1 : if (Start == 1'b1) begin
			state <= 2;
			e = a / b;
			f = c / d;
			g = a % b  ;
			end
		2 : if (Start == 1'b1) begin
			state <= 3;

		3 : if (Start == 1'b1) begin
			state <= 4;

		4 : if (Start == 1'b1) begin
			state <= 5;
			gEQz = g == zero;
			end
		5 : if (Start == 1'b1) begin
			state <= 6;
			z = gEQz ? e : f ;
			end
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

		11 : if (Start == 1'b1) begin
			state <= 0;
			Done <= 1; 
			end
		default : state <= 0; 
	endcase 
end
end 
endmodule 