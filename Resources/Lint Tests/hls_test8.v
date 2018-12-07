`timescale 1ns/1ns

module HLSM(Clk, Rst, Start, x0, x1, x2, x3, y0, c0, five, ten, d1, d2, e, f, g, h, Done);
    input                    Clk, Rst, Start;
    output reg               Done;
    input signed [31:0]      x0, x1, x2, x3, y0, c0, five, ten;
    output reg signed [31:0] d1, d2, e, f, g, h;
    reg signed [31:0]        t1, t2, t3, vd1, ve, vf, vg, state;

    always @(posedge Clk) begin
        if (Rst == 1'b1) begin
            state <= 0;
            d1 <= 0;
            d2 <= 0;
            e <= 0;
            f <= 0;
            g <= 0;
            h <= 0;
        end else begin
            case (state)
                0: if (Start == 1'b1) begin
                    state <= 1;
                    Done <= 0;
                    d1 <= 0;
                    d2 <= 0;
                    e <= 0;
                    f <= 0;
                    g <= 0;
                    h <= 0;
                end else begin
                    state <= 0;
                end
                1: if (Start == 1'b1) begin
                    state <= 2;
                    t1 = x0+x1;
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
