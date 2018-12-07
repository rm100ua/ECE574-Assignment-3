`timescale 1ns/1ns

module HLSM(Clk, Rst, Start, a, b, c, d, e, f, g, h, sa, one, two, four, avg, Done);
    input                    Clk, Rst, Start;
    output reg               Done;
    input signed [31:0]      a, b, c, d, e, f, g, h, sa, one, two, four;
    output reg signed [31:0] avg;
    reg signed [31:0]        t1, t2, t3, t4, t5, t6, t7, t7div2, t7div4, state;
    reg signed [0:0]         csa1, csa2, csa3, state;

    always @(posedge Clk) begin
        if (Rst == 1'b1) begin
            state <= 0;
            avg <= 0;
        end else begin
            case (state)
                0: if (Start == 1'b1) begin
                    state <= 1;
                    Done <= 0;
                    avg <= 0;
                end else begin
                    state <= 0;
                end
                1: if (Start == 1'b1) begin
                    state <= 2;
                    t1 = a+b;
                end
                2: if (Start == 1'b1) begin
                    state <= 3;

                    3:if (Start == 1'b1) begin
                        state <= 4;

                        4:if (Start == 1'b1) begin
                            state <= 5;

                            5:if (Start == 1'b1) begin
                                state <= 6;

                                6:if (Start == 1'b1) begin
                                    state <= 7;

                                    7:if (Start == 1'b1) begin
                                        state <= 8;

                                        8:if (Start == 1'b1) begin
                                            state <= 9;

                                            9:if (Start == 1'b1) begin
                                                state <= 10;

                                                10:if (Start == 1'b1) begin
                                                    state <= 11;

                                                    11:if (Start == 1'b1) begin
                                                        state <= 0;
                                                        Done <= 1;
                                                    end
                                                    default:state <= 0;
                                                    endcase
                                                end
                                            end
                                            endmodule
