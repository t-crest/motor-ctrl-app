	component DOC_Monitor is
		port (
			in_port_to_the_IO_IN_Buttons   : in  std_logic_vector(3 downto 0)  := (others => 'X'); -- export
			pio_pfc_in_port                : in  std_logic_vector(1 downto 0)  := (others => 'X'); -- in_port
			pio_pfc_out_port               : out std_logic_vector(1 downto 0);                     -- out_port
			dc_link_sync_dat               : in  std_logic                     := 'X';             -- sync_dat
			dc_link_dc_link_enable         : in  std_logic                     := 'X';             -- dc_link_enable
			dc_link_overvoltage            : out std_logic;                                        -- overvoltage
			dc_link_undervoltage           : out std_logic;                                        -- undervoltage
			dc_link_chopper                : out std_logic;                                        -- chopper
			dc_link_p_sync_dat             : in  std_logic                     := 'X';             -- sync_dat
			dc_link_p_dc_link_enable       : in  std_logic                     := 'X';             -- dc_link_enable
			dc_link_p_overvoltage          : out std_logic;                                        -- overvoltage
			dc_link_p_undervoltage         : out std_logic;                                        -- undervoltage
			dc_link_p_chopper              : out std_logic;                                        -- chopper
			clk_adc_in_clk                 : in  std_logic                     := 'X';             -- clk
			avs_periph_slave_waitrequest   : out std_logic;                                        -- waitrequest
			avs_periph_slave_readdata      : out std_logic_vector(31 downto 0);                    -- readdata
			avs_periph_slave_readdatavalid : out std_logic;                                        -- readdatavalid
			avs_periph_slave_burstcount    : in  std_logic_vector(0 downto 0)  := (others => 'X'); -- burstcount
			avs_periph_slave_writedata     : in  std_logic_vector(31 downto 0) := (others => 'X'); -- writedata
			avs_periph_slave_address       : in  std_logic_vector(11 downto 0) := (others => 'X'); -- address
			avs_periph_slave_write         : in  std_logic                     := 'X';             -- write
			avs_periph_slave_read          : in  std_logic                     := 'X';             -- read
			avs_periph_slave_byteenable    : in  std_logic_vector(3 downto 0)  := (others => 'X'); -- byteenable
			avs_periph_slave_debugaccess   : in  std_logic                     := 'X';             -- debugaccess
			clk_80_clk                     : in  std_logic                     := 'X';             -- clk
			reset_80_reset_n               : in  std_logic                     := 'X';             -- reset_n
			clk_50_clk                     : in  std_logic                     := 'X';             -- clk
			reset_50_reset_n               : in  std_logic                     := 'X'              -- reset_n
		);
	end component DOC_Monitor;

	u0 : component DOC_Monitor
		port map (
			in_port_to_the_IO_IN_Buttons   => CONNECTED_TO_in_port_to_the_IO_IN_Buttons,   -- io_in_buttons_external_connection.export
			pio_pfc_in_port                => CONNECTED_TO_pio_pfc_in_port,                --                           pio_pfc.in_port
			pio_pfc_out_port               => CONNECTED_TO_pio_pfc_out_port,               --                                  .out_port
			dc_link_sync_dat               => CONNECTED_TO_dc_link_sync_dat,               --                           dc_link.sync_dat
			dc_link_dc_link_enable         => CONNECTED_TO_dc_link_dc_link_enable,         --                                  .dc_link_enable
			dc_link_overvoltage            => CONNECTED_TO_dc_link_overvoltage,            --                                  .overvoltage
			dc_link_undervoltage           => CONNECTED_TO_dc_link_undervoltage,           --                                  .undervoltage
			dc_link_chopper                => CONNECTED_TO_dc_link_chopper,                --                                  .chopper
			dc_link_p_sync_dat             => CONNECTED_TO_dc_link_p_sync_dat,             --                         dc_link_p.sync_dat
			dc_link_p_dc_link_enable       => CONNECTED_TO_dc_link_p_dc_link_enable,       --                                  .dc_link_enable
			dc_link_p_overvoltage          => CONNECTED_TO_dc_link_p_overvoltage,          --                                  .overvoltage
			dc_link_p_undervoltage         => CONNECTED_TO_dc_link_p_undervoltage,         --                                  .undervoltage
			dc_link_p_chopper              => CONNECTED_TO_dc_link_p_chopper,              --                                  .chopper
			clk_adc_in_clk                 => CONNECTED_TO_clk_adc_in_clk,                 --                        clk_adc_in.clk
			avs_periph_slave_waitrequest   => CONNECTED_TO_avs_periph_slave_waitrequest,   --                  avs_periph_slave.waitrequest
			avs_periph_slave_readdata      => CONNECTED_TO_avs_periph_slave_readdata,      --                                  .readdata
			avs_periph_slave_readdatavalid => CONNECTED_TO_avs_periph_slave_readdatavalid, --                                  .readdatavalid
			avs_periph_slave_burstcount    => CONNECTED_TO_avs_periph_slave_burstcount,    --                                  .burstcount
			avs_periph_slave_writedata     => CONNECTED_TO_avs_periph_slave_writedata,     --                                  .writedata
			avs_periph_slave_address       => CONNECTED_TO_avs_periph_slave_address,       --                                  .address
			avs_periph_slave_write         => CONNECTED_TO_avs_periph_slave_write,         --                                  .write
			avs_periph_slave_read          => CONNECTED_TO_avs_periph_slave_read,          --                                  .read
			avs_periph_slave_byteenable    => CONNECTED_TO_avs_periph_slave_byteenable,    --                                  .byteenable
			avs_periph_slave_debugaccess   => CONNECTED_TO_avs_periph_slave_debugaccess,   --                                  .debugaccess
			clk_80_clk                     => CONNECTED_TO_clk_80_clk,                     --                            clk_80.clk
			reset_80_reset_n               => CONNECTED_TO_reset_80_reset_n,               --                          reset_80.reset_n
			clk_50_clk                     => CONNECTED_TO_clk_50_clk,                     --                            clk_50.clk
			reset_50_reset_n               => CONNECTED_TO_reset_50_reset_n                --                          reset_50.reset_n
		);

