`timescale 1ns / 1ns 

module HLSM (Clk, Rst, Start, a, b, c, d, e, f, g, j, l, Done); 
input Clk, Rst, Start; 
output reg Done; 
input signed [15:0] a, b, c, d, e, f, g; 
reg signed [15:0] h, i, k, state; 
output reg signed [15:0] j, l; 

always@(posedge Clk) begin 
if (Rst == 1'b1) begin 
	state <= 0; 
	j <= 0; 
	l <= 0; 
end else begin 
	case(state) 
		0 : if (Start == 1'b1) begin
			state <= 1;
			Done <= 0; 
			j <= 0; 
			l <= 0; 
		end else begin 
			state <= 0; 
		end
		1 : if (Start == 1'b1) begin
			state <= 2;
			h = a + b;
			k = e * f;
			end
		2 : if (Start == 1'b1) begin
			state <= 3;

		3 : if (Start == 1'b1) begin
			state <= 4;
			i = h + c ;
			j = i + d ;
			l = k * g;
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

		11 : if (Start == 1'b1) begin
			state <= 0;
			Done <= 1; 
			end
		default : state <= 0; 
	endcase 
end
end 
endmodule 