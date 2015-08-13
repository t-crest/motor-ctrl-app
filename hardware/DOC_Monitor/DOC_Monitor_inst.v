	DOC_Monitor u0 (
		.in_port_to_the_IO_IN_Buttons   (<connected-to-in_port_to_the_IO_IN_Buttons>),   // io_in_buttons_external_connection.export
		.pio_pfc_in_port                (<connected-to-pio_pfc_in_port>),                //                           pio_pfc.in_port
		.pio_pfc_out_port               (<connected-to-pio_pfc_out_port>),               //                                  .out_port
		.dc_link_sync_dat               (<connected-to-dc_link_sync_dat>),               //                           dc_link.sync_dat
		.dc_link_dc_link_enable         (<connected-to-dc_link_dc_link_enable>),         //                                  .dc_link_enable
		.dc_link_overvoltage            (<connected-to-dc_link_overvoltage>),            //                                  .overvoltage
		.dc_link_undervoltage           (<connected-to-dc_link_undervoltage>),           //                                  .undervoltage
		.dc_link_chopper                (<connected-to-dc_link_chopper>),                //                                  .chopper
		.dc_link_p_sync_dat             (<connected-to-dc_link_p_sync_dat>),             //                         dc_link_p.sync_dat
		.dc_link_p_dc_link_enable       (<connected-to-dc_link_p_dc_link_enable>),       //                                  .dc_link_enable
		.dc_link_p_overvoltage          (<connected-to-dc_link_p_overvoltage>),          //                                  .overvoltage
		.dc_link_p_undervoltage         (<connected-to-dc_link_p_undervoltage>),         //                                  .undervoltage
		.dc_link_p_chopper              (<connected-to-dc_link_p_chopper>),              //                                  .chopper
		.clk_adc_in_clk                 (<connected-to-clk_adc_in_clk>),                 //                        clk_adc_in.clk
		.avs_periph_slave_waitrequest   (<connected-to-avs_periph_slave_waitrequest>),   //                  avs_periph_slave.waitrequest
		.avs_periph_slave_readdata      (<connected-to-avs_periph_slave_readdata>),      //                                  .readdata
		.avs_periph_slave_readdatavalid (<connected-to-avs_periph_slave_readdatavalid>), //                                  .readdatavalid
		.avs_periph_slave_burstcount    (<connected-to-avs_periph_slave_burstcount>),    //                                  .burstcount
		.avs_periph_slave_writedata     (<connected-to-avs_periph_slave_writedata>),     //                                  .writedata
		.avs_periph_slave_address       (<connected-to-avs_periph_slave_address>),       //                                  .address
		.avs_periph_slave_write         (<connected-to-avs_periph_slave_write>),         //                                  .write
		.avs_periph_slave_read          (<connected-to-avs_periph_slave_read>),          //                                  .read
		.avs_periph_slave_byteenable    (<connected-to-avs_periph_slave_byteenable>),    //                                  .byteenable
		.avs_periph_slave_debugaccess   (<connected-to-avs_periph_slave_debugaccess>),   //                                  .debugaccess
		.clk_80_clk                     (<connected-to-clk_80_clk>),                     //                            clk_80.clk
		.reset_80_reset_n               (<connected-to-reset_80_reset_n>),               //                          reset_80.reset_n
		.clk_50_clk                     (<connected-to-clk_50_clk>),                     //                            clk_50.clk
		.reset_50_reset_n               (<connected-to-reset_50_reset_n>)                //                          reset_50.reset_n
	);

