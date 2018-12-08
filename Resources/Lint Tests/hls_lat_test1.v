`timescale 1ns / 1ns 

module HLSM (Clk, Rst, Start, a, b, c, d, e, i, Done); 
input Clk, Rst, Start; 
output reg Done; 
input signed [15:0] a, b, c, d, e; 
output reg signed [15:0] i; 
reg signed [15:0] f, g, h, state; 

always@(posedge Clk) begin 
if (Rst == 1'b1) begin 
	state <= 0; 
	i <= 0; 
end else begin 
	case(state) 
		0 : if (Start == 1'b1) begin
			state <= 1;
			Done <= 0; 
			i <= 0; 
		end else begin 
			state <= 0; 
		end
		1 : if (Start == 1'b1) begin
			state <= 2;
			f = a + b;
			end
		2 : if (Start == 1'b1) begin
			state <= 3;

		3 : if (Start == 1'b1) begin
			state <= 4;
			g = f + c;
			h = g + d;
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
			i = h + e;
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
