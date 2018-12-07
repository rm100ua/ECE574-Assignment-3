`timescale 1ns/1ns

module HLSM(Clk, Rst, Start, go, a, b, c, start, inc, status, notdone, z, x, Done);
    input                    Clk, Rst, Start;
    output reg               Done;
    input signed [0:0]       go;
    input signed [31:0]      a, b, c, start, inc;
    input        [1:0]       status, notdone;
    output reg signed [31:0] z, x;
    reg signed [31:0]        e, g, d, f, count, state;

    always @(posedge Clk) begin
        if (Rst == 1'b1) begin
            state <= 0;
            z <= 0;
            x <= 0;
        end else begin
            case (state)
                0: if (Start == 1'b1) begin
                    state <= 1;
                    Done <= 0;
                    z <= 0;
                    x <= 0;
                end else begin
                    state <= 0;
                end
                1: if (Start == 1'b1) begin
                    state <= 2;
                    for (count = start; status == notdone;  count = count*inc)
                {;
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
