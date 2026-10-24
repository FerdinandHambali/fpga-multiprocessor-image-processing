// ID: 35492694, 32920474, 33931097 and 33531153
module milestone2_top (
    input         CLOCK_50,
    input  [9:0]  SW,
    input  [1:0]  KEY,

    output [9:0]  LEDR,
    output [7:0]  HEX0,
    output [7:0]  HEX1,
    output [7:0]  HEX2,
    output [7:0]  HEX3,
    output [7:0]  HEX4,
    output [7:0]  HEX5,

    output [12:0] DRAM_ADDR,
    output [1:0]  DRAM_BA,
    output        DRAM_CAS_N,
    output        DRAM_CKE,
    output        DRAM_CLK,
    output        DRAM_CS_N,
    inout  [15:0] DRAM_DQ,
    output        DRAM_LDQM,
    output        DRAM_RAS_N,
    output        DRAM_UDQM,
    output        DRAM_WE_N,

    output [3:0]  VGA_R,
    output [3:0]  VGA_G,
    output [3:0]  VGA_B,
    output        VGA_HS,
    output        VGA_VS,
	 
	 //SPI 
	 inout [31:0] GPIO,
	 
	 //Accelerometer
	 input GSENSOR_SDO,
	 output GSENSOR_SDI,
	 output GSENSOR_CS_N,
	 output GSENSOR_SCLK,
	 input [2:1] GSENSOR_INT
);

	 //Counter
	 wire [31:0] usec_wire;
	 usec_counter counter_inst(
		.clk(CLOCK_50),
		.reset_n(KEY[0]),
		.usec_count(usec_wire)
	 );
	 
	 //SPI
	 wire spi_MISO;
	 wire spi_MOSI;
	 wire spi_SCLK;
	 wire [1:0] spi_select;
	 wire cam_miso;
	 wire acc_miso;
	 //miso
	 assign cam_miso = GPIO[7]; //SDO camera
	 assign acc_miso = GSENSOR_SDO;
	 //mosi
	 assign GPIO[8] = spi_MOSI; //SDI camera
	 //clock
	 assign GPIO[9] = spi_SCLK; //clock for camera 
	 //select
	 assign GPIO[5] = spi_select[0];
	 assign GPIO[31:10] = 22'bz;
	 assign GPIO[6] = 1'bz;
	 assign GPIO[4:3] = 1'bz;
	 assign GPIO[1:0] = 2'bz;
	 //Accelerometer
	 assign GSENSOR_CS_N = spi_select[1];
	 assign GSENSOR_SDI = spi_MOSI;
	 assign GSENSOR_SCLK = spi_SCLK;
	 
	 //1 slave at a time for miso -> same bus
	 assign spi_MISO =
	   (spi_select[0] == 1'b0) ? cam_miso :
	   (spi_select[1] == 1'b0) ? acc_miso :
	   1'b1;

    // VGA PLL
    wire clk_25;
    wire pll_locked;

    vga_pll pll_inst (
        .inclk0 (CLOCK_50),
        .c0     (clk_25),
        .locked (pll_locked)
    );
	 
    //SDRAM
	 wire sdram_clk;
	 assign DRAM_CLK = sdram_clk;

    sdram_pll sdram_inst (
        .inclk0 (CLOCK_50),
        .c0     (sdram_clk)
    );
	 
    // =========================================================
    // Wires from Platform Designer system
    // =========================================================
    wire [16:0] pix_address;
    wire [3:0]  pix_data;
    wire        pix_we;

    wire [9:0]  led_wire;
    wire [23:0] hex0_wire;
	 wire [23:0] hex1_wire;

    wire [11:0] dram_addr_int;
    wire [1:0]  dram_dqm;

	 //Camera
    wire cam_ready;
    assign cam_ready = GPIO[2];
	
    // VGA / pixel buffer wires
    wire [18:0] VGA_ADDR;
    wire [16:0] pixel_rdaddr;
    wire [3:0]  pixel_rdata;

    assign pixel_rdaddr = VGA_ADDR[16:0];

    // Platform Designer->12 address bits, but board has 13 pins
    assign DRAM_ADDR[11:0] = dram_addr_int;
    assign DRAM_ADDR[12]   = 1'b0;
    assign DRAM_LDQM = dram_dqm[0];
    assign DRAM_UDQM = dram_dqm[1];

    //HEX and LED outputs
    assign LEDR = led_wire;
    assign HEX0 = hex0_wire[7:0];
    assign HEX1 = hex0_wire[15:8];
    assign HEX2 = hex0_wire[23:16];
    assign HEX3 = hex1_wire[7:0];
    assign HEX4 = hex1_wire[15:8];
    assign HEX5 = hex1_wire[23:16];

    //Platform Designer system(MULTIPROCESSOR)

    multi_proc0 u0 (
        .clk_clk             (CLOCK_50),

        // communication side
        .camera_ready_export (cam_ready),
        .spi_MISO            (spi_MISO),
        .spi_MOSI            (spi_MOSI),
        .spi_SCLK            (spi_SCLK),
        .spi_SS_n            (spi_select),
        .accelerometer_export(GSENSOR_INT[2]),

        // SDRAM shared
        .sdram_addr          (dram_addr_int),
        .sdram_ba            (DRAM_BA),
        .sdram_cas_n         (DRAM_CAS_N),
        .sdram_cke           (DRAM_CKE),
        .sdram_cs_n          (DRAM_CS_N),
        .sdram_dq            (DRAM_DQ),
        .sdram_dqm           (dram_dqm),
        .sdram_ras_n         (DRAM_RAS_N),
        .sdram_we_n          (DRAM_WE_N),

        // processing side
        .pixel_output_export (pix_data),
        .pixel_send_export   (pix_address),
        .pixel_we_external_connection_export (pix_we),

        .switch_export       (SW),
        .key_export          (KEY),
        .ledr_export         (led_wire),
        .hex0_export         (hex0_wire),
        .hex5_export         (hex1_wire),
        .counter_export      (usec_wire)
    );

    //Pixel buffer
    pixel_buffer buf_inst (
        .clock     (clk_25),
        .data      (pix_data),
        .rdaddress (pixel_rdaddr),
        .rden      (1'b1),
        .wraddress (pix_address),
        .wren      (pix_we),
        .q         (pixel_rdata)
    );

    //VGA controller
    vga_controller vga_inst (
        .VGA_CLK  (clk_25),
        .VGA_DATA (pixel_rdata),
        .VGA_ADDR (VGA_ADDR),
        .VGA_R    (VGA_R),
        .VGA_G    (VGA_G),
        .VGA_B    (VGA_B),
        .VGA_HS   (VGA_HS),
        .VGA_VS   (VGA_VS)
    );

endmodule