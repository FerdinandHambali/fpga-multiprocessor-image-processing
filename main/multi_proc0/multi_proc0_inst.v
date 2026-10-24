	multi_proc0 u0 (
		.accelerometer_export                (<connected-to-accelerometer_export>),                //                accelerometer.export
		.camera_ready_export                 (<connected-to-camera_ready_export>),                 //                 camera_ready.export
		.clk_clk                             (<connected-to-clk_clk>),                             //                          clk.clk
		.counter_export                      (<connected-to-counter_export>),                      //                      counter.export
		.hex0_export                         (<connected-to-hex0_export>),                         //                         hex0.export
		.hex5_export                         (<connected-to-hex5_export>),                         //                         hex5.export
		.key_export                          (<connected-to-key_export>),                          //                          key.export
		.ledr_export                         (<connected-to-ledr_export>),                         //                         ledr.export
		.pixel_output_export                 (<connected-to-pixel_output_export>),                 //                 pixel_output.export
		.pixel_send_export                   (<connected-to-pixel_send_export>),                   //                   pixel_send.export
		.pixel_we_external_connection_export (<connected-to-pixel_we_external_connection_export>), // pixel_we_external_connection.export
		.sdram_addr                          (<connected-to-sdram_addr>),                          //                        sdram.addr
		.sdram_ba                            (<connected-to-sdram_ba>),                            //                             .ba
		.sdram_cas_n                         (<connected-to-sdram_cas_n>),                         //                             .cas_n
		.sdram_cke                           (<connected-to-sdram_cke>),                           //                             .cke
		.sdram_cs_n                          (<connected-to-sdram_cs_n>),                          //                             .cs_n
		.sdram_dq                            (<connected-to-sdram_dq>),                            //                             .dq
		.sdram_dqm                           (<connected-to-sdram_dqm>),                           //                             .dqm
		.sdram_ras_n                         (<connected-to-sdram_ras_n>),                         //                             .ras_n
		.sdram_we_n                          (<connected-to-sdram_we_n>),                          //                             .we_n
		.spi_MISO                            (<connected-to-spi_MISO>),                            //                          spi.MISO
		.spi_MOSI                            (<connected-to-spi_MOSI>),                            //                             .MOSI
		.spi_SCLK                            (<connected-to-spi_SCLK>),                            //                             .SCLK
		.spi_SS_n                            (<connected-to-spi_SS_n>),                            //                             .SS_n
		.switch_export                       (<connected-to-switch_export>)                        //                       switch.export
	);

