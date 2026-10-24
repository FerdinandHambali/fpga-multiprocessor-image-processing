module top_pixel_buffer (
    input  CLOCK_50,        // 50 MHz from DE10-Nano
    input  [3:0] SW,        // switches: pixel data for test writes
    input  [1:0] KEY,       // KEY[0]: write enable, KEY[1]: increment address (active low)
    output [3:0] VGA_R,
    output [3:0] VGA_G,
    output [3:0] VGA_B,
    output       VGA_HS,
    output       VGA_VS
);

// -----------------------------------------------------------------
// PLL: generate 25 MHz clock
// -----------------------------------------------------------------
wire clk_25;
wire locked;

vga_pll pll_inst (
    .inclk0(CLOCK_50),
    .c0(clk_25),
    .locked(locked)
);

// -----------------------------------------------------------------
// Reset for logic (active high)
// -----------------------------------------------------------------
wire rst = !locked;   // keep system reset until PLL locks

// -----------------------------------------------------------------
// Pixel buffer signals
// -----------------------------------------------------------------
wire [3:0]  pixel_data_w;      // write data from test logic (later from Nios)
wire [16:0] pixel_wraddr;      // write address
wire        pixel_wren;        // write enable

wire [3:0]  pixel_rdata;       // read data to VGA controller
wire [16:0] pixel_rdaddr;      // read address from VGA controller

// Connect read side: always enabled
assign pixel_rdaddr = VGA_ADDR[16:0];   // VGA_ADDR from vga_controller

// -----------------------------------------------------------------
// Pixel buffer instance
// -----------------------------------------------------------------
pixel_buffer buf_inst (
    .clock(clk_25),
    .data(pixel_data_w),
    .rdaddress(pixel_rdaddr),
    .rden(1'b1),                 // always read
    .wraddress(pixel_wraddr),
    .wren(pixel_wren),
    .q(pixel_rdata)
);

// -----------------------------------------------------------------
// VGA controller instance
// -----------------------------------------------------------------
wire [18:0] VGA_ADDR;    // from VGA controller

vga_controller vga_inst (
    .VGA_CLK(clk_25),
    .VGA_DATA(pixel_rdata),
    .VGA_ADDR(VGA_ADDR),
    .VGA_R(VGA_R),
    .VGA_G(VGA_G),
    .VGA_B(VGA_B),
    .VGA_HS(VGA_HS),
    .VGA_VS(VGA_VS)
);

// -----------------------------------------------------------------
// Test logic: write pixels using switches and keys
// -----------------------------------------------------------------
reg [16:0] test_addr;
reg [3:0]  test_data;
reg        test_we;

always @(posedge clk_25 or posedge rst) begin
    if (rst) begin
        test_addr <= 0;
        test_data <= 0;
        test_we   <= 0;
    end else begin
        // Write when KEY[0] is pressed (active low)
        test_we <= ~KEY[0];          // write enable = 1 when key pressed
        test_data <= SW;              // pixel value from switches
        // Increment address when KEY[1] is pressed (active low)
        if (~KEY[1])
            test_addr <= test_addr + 1;
    end
end

// Connect test logic to pixel buffer write ports
assign pixel_data_w = test_data;
assign pixel_wraddr = test_addr;
assign pixel_wren   = test_we;

endmodule