	component multi_proc0 is
		port (
			accelerometer_export                : in    std_logic                     := 'X';             -- export
			camera_ready_export                 : in    std_logic                     := 'X';             -- export
			clk_clk                             : in    std_logic                     := 'X';             -- clk
			counter_export                      : in    std_logic_vector(31 downto 0) := (others => 'X'); -- export
			hex0_export                         : out   std_logic_vector(23 downto 0);                    -- export
			hex5_export                         : out   std_logic_vector(23 downto 0);                    -- export
			key_export                          : in    std_logic_vector(1 downto 0)  := (others => 'X'); -- export
			ledr_export                         : out   std_logic_vector(9 downto 0);                     -- export
			pixel_output_export                 : out   std_logic_vector(3 downto 0);                     -- export
			pixel_send_export                   : out   std_logic_vector(16 downto 0);                    -- export
			pixel_we_external_connection_export : out   std_logic;                                        -- export
			sdram_addr                          : out   std_logic_vector(12 downto 0);                    -- addr
			sdram_ba                            : out   std_logic_vector(1 downto 0);                     -- ba
			sdram_cas_n                         : out   std_logic;                                        -- cas_n
			sdram_cke                           : out   std_logic;                                        -- cke
			sdram_cs_n                          : out   std_logic;                                        -- cs_n
			sdram_dq                            : inout std_logic_vector(15 downto 0) := (others => 'X'); -- dq
			sdram_dqm                           : out   std_logic_vector(1 downto 0);                     -- dqm
			sdram_ras_n                         : out   std_logic;                                        -- ras_n
			sdram_we_n                          : out   std_logic;                                        -- we_n
			spi_MISO                            : in    std_logic                     := 'X';             -- MISO
			spi_MOSI                            : out   std_logic;                                        -- MOSI
			spi_SCLK                            : out   std_logic;                                        -- SCLK
			spi_SS_n                            : out   std_logic_vector(1 downto 0);                     -- SS_n
			switch_export                       : in    std_logic_vector(9 downto 0)  := (others => 'X')  -- export
		);
	end component multi_proc0;

	u0 : component multi_proc0
		port map (
			accelerometer_export                => CONNECTED_TO_accelerometer_export,                --                accelerometer.export
			camera_ready_export                 => CONNECTED_TO_camera_ready_export,                 --                 camera_ready.export
			clk_clk                             => CONNECTED_TO_clk_clk,                             --                          clk.clk
			counter_export                      => CONNECTED_TO_counter_export,                      --                      counter.export
			hex0_export                         => CONNECTED_TO_hex0_export,                         --                         hex0.export
			hex5_export                         => CONNECTED_TO_hex5_export,                         --                         hex5.export
			key_export                          => CONNECTED_TO_key_export,                          --                          key.export
			ledr_export                         => CONNECTED_TO_ledr_export,                         --                         ledr.export
			pixel_output_export                 => CONNECTED_TO_pixel_output_export,                 --                 pixel_output.export
			pixel_send_export                   => CONNECTED_TO_pixel_send_export,                   --                   pixel_send.export
			pixel_we_external_connection_export => CONNECTED_TO_pixel_we_external_connection_export, -- pixel_we_external_connection.export
			sdram_addr                          => CONNECTED_TO_sdram_addr,                          --                        sdram.addr
			sdram_ba                            => CONNECTED_TO_sdram_ba,                            --                             .ba
			sdram_cas_n                         => CONNECTED_TO_sdram_cas_n,                         --                             .cas_n
			sdram_cke                           => CONNECTED_TO_sdram_cke,                           --                             .cke
			sdram_cs_n                          => CONNECTED_TO_sdram_cs_n,                          --                             .cs_n
			sdram_dq                            => CONNECTED_TO_sdram_dq,                            --                             .dq
			sdram_dqm                           => CONNECTED_TO_sdram_dqm,                           --                             .dqm
			sdram_ras_n                         => CONNECTED_TO_sdram_ras_n,                         --                             .ras_n
			sdram_we_n                          => CONNECTED_TO_sdram_we_n,                          --                             .we_n
			spi_MISO                            => CONNECTED_TO_spi_MISO,                            --                          spi.MISO
			spi_MOSI                            => CONNECTED_TO_spi_MOSI,                            --                             .MOSI
			spi_SCLK                            => CONNECTED_TO_spi_SCLK,                            --                             .SCLK
			spi_SS_n                            => CONNECTED_TO_spi_SS_n,                            --                             .SS_n
			switch_export                       => CONNECTED_TO_switch_export                        --                       switch.export
		);

