
module multi_proc0 (
	accelerometer_export,
	camera_ready_export,
	clk_clk,
	counter_export,
	hex0_export,
	hex5_export,
	key_export,
	ledr_export,
	pixel_output_export,
	pixel_send_export,
	pixel_we_external_connection_export,
	sdram_addr,
	sdram_ba,
	sdram_cas_n,
	sdram_cke,
	sdram_cs_n,
	sdram_dq,
	sdram_dqm,
	sdram_ras_n,
	sdram_we_n,
	spi_MISO,
	spi_MOSI,
	spi_SCLK,
	spi_SS_n,
	switch_export);	

	input		accelerometer_export;
	input		camera_ready_export;
	input		clk_clk;
	input	[31:0]	counter_export;
	output	[23:0]	hex0_export;
	output	[23:0]	hex5_export;
	input	[1:0]	key_export;
	output	[9:0]	ledr_export;
	output	[3:0]	pixel_output_export;
	output	[16:0]	pixel_send_export;
	output		pixel_we_external_connection_export;
	output	[12:0]	sdram_addr;
	output	[1:0]	sdram_ba;
	output		sdram_cas_n;
	output		sdram_cke;
	output		sdram_cs_n;
	inout	[15:0]	sdram_dq;
	output	[1:0]	sdram_dqm;
	output		sdram_ras_n;
	output		sdram_we_n;
	input		spi_MISO;
	output		spi_MOSI;
	output		spi_SCLK;
	output	[1:0]	spi_SS_n;
	input	[9:0]	switch_export;
endmodule
