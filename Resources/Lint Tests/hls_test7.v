`timescale 1ns / 1ns 

module HLSM (Clk, Rst, Start, u, x, y, dx, a, three, u1, x1, y1, c, Done); 
input Clk, Rst, Start; 
output reg Done; 
input signed [31:0] u, x, y, dx, a, three; 
output reg signed [31:0] u1, x1, y1, c; 
reg signed [31:0] t1, t2, t3, t4, t5, t6, t7, vx1, state; 

always@(posedge Clk) begin 
if (Rst == 1'b1) begin 
	state <= 0; 
	u1 <= 0; 
	x1 <= 0; 
	y1 <= 0; 
	c <= 0; 
end else begin 
	case(state) 
		0 : if (Start == 1'b1) begin
			state <= 1;
			Done <= 0; 
			u1 <= 0; 
			x1 <= 0; 
			y1 <= 0; 
			c <= 0; 
		end else begin 
			state <= 0; 
		end
		1 : if (Start == 1'b1) begin
			state <= 2;
			x1 = x + dx;
			vx1 = x + dx;
			t1 = three * x;
			t2 = u * dx;
			t5 = three * y;
			t7 = u * dx;
			end
		2 : if (Start == 1'b1) begin
			state <= 3;

		3 : if (Start == 1'b1) begin
			state <= 4;
			t3 = t1 * t2;
			t6 = t5 * dx;
			y1 = y + t7;
			c = vx1 < a;
			end
		4 : if (Start == 1'b1) begin
			state <= 5;

		5 : if (Start == 1'b1) begin
			state <= 6;

		6 : if (Start == 1'b1) begin
			state <= 7;

		7 : if (Start == 1'b1) begin
			state <= 8;
			t4 = u - t3;
			u1 = t4 - t6;
			end
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
