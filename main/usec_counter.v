// ID: 32920474
module usec_counter ( 
    input        clk,
    input        reset_n,
    output reg [31:0] usec_count //use it in eclipse
);

    reg [5:0] cycle_count;  
    //49 counts -> 6 bits (since the clock is 50 MHz, i made it to loop 49 times)

    always @(posedge clk or negedge reset_n) begin
        if (!reset_n) begin
            cycle_count <= 6'd0;
            usec_count  <= 32'd0;
        end
        else begin
            if (cycle_count == 6'd49) begin
                cycle_count <= 6'd0; //reset cycle count if it has already reached 59 clock cycles)
                usec_count  <= usec_count + 32'd1; //each loop will increment the counter by 1
            end
            else begin
                cycle_count <= cycle_count + 6'd1;
            end
        end
    end

endmodule