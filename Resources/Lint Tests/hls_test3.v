`timescale 1ns / 1ns 

module HLSM (Clk, Rst, Start, a, b, c, d, e, f, g, h, num, avg, Done); 
input Clk, Rst, Start; 
output reg Done; 
input signed [7:0] a, b, c, d, e, f, g, h, num; 
output reg signed [7:0] avg; 
reg signed [31:0] t1, t2, t3, t4, t5, t6, t7, state; 

always@(posedge Clk) begin 
if (Rst == 1'b1) begin 
	state <= 0; 
	avg <= 0; 
end else begin 
	case(state) 
		0 : if (Start == 1'b1) begin
			state <= 1;
			Done <= 0; 
			avg <= 0; 
		end else begin 
			state <= 0; 
		end
		1 : if (Start == 1'b1) begin
			state <= 2;
			t1 = a + b;
			end
		2 : if (Start == 1'b1) begin
			state <= 3;
			t2 = t1 + c ;
			end
		3 : if (Start == 1'b1) begin
			state <= 4;
			t3 = t2 + d ;
			end
		4 : if (Start == 1'b1) begin
			state <= 5;
			t4 = t3 + e ;
			end
		5 : if (Start == 1'b1) begin
			state <= 6;
			t5 = t4 + f ;
			end
		6 : if (Start == 1'b1) begin
			state <= 7;
			t6 = t5 + g ;
			end
		7 : if (Start == 1'b1) begin
			state <= 8;
			t7 = t6 + h ;
			end
		8 : if (Start == 1'b1) begin
			state <= 9;
			avg = t7 / num;
			end
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
