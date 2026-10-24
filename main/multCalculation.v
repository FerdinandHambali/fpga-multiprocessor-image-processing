//ID:  33931097
module multCalculation (
    input  [31:0] dataa,
    input  [31:0] datab,
    output [31:0] result
);

    wire [3:0] p_tl;
    wire [3:0] p_t;
    wire [3:0] p_tr;
    wire [3:0] p_l;
    wire [3:0] p_r;
    wire [3:0] p_bl;
    wire [3:0] p_b;
    wire [3:0] p_br;

    wire signed [8:0] gx;
    wire signed [8:0] gy;

    wire [8:0] abs_gx;
    wire [8:0] abs_gy;
    wire [9:0] strength;

    wire [7:0] threshold;
    wire [3:0] edge_pixel;

    assign p_tl = dataa[3:0];
    assign p_t  = dataa[7:4];
    assign p_tr = dataa[11:8];
    assign p_l  = dataa[15:12];
    assign p_r  = dataa[19:16];
    assign p_bl = dataa[23:20];
    assign p_b  = dataa[27:24];
    assign p_br = dataa[31:28];

    assign threshold = datab[7:0];

    /*
        Sobel X:
        -1  0  1
        -2  0  2
        -1  0  1

        Sobel Y:
        -1 -2 -1
         0  0  0
         1  2  1
    */

    assign gx =
        -$signed({5'b0, p_tl}) +
         $signed({5'b0, p_tr}) -
        ($signed({5'b0, p_l})  <<< 1) +
        ($signed({5'b0, p_r})  <<< 1) -
         $signed({5'b0, p_bl}) +
         $signed({5'b0, p_br});

    assign gy =
        -$signed({5'b0, p_tl}) -
        ($signed({5'b0, p_t})  <<< 1) -
         $signed({5'b0, p_tr}) +
         $signed({5'b0, p_bl}) +
        ($signed({5'b0, p_b})  <<< 1) +
         $signed({5'b0, p_br});

    assign abs_gx = gx[8] ? -gx : gx;
    assign abs_gy = gy[8] ? -gy : gy;

    assign strength = abs_gx + abs_gy;

    assign edge_pixel = (strength > threshold) ? 4'hF : 4'h0;

    assign result = {28'b0, edge_pixel};

endmodule