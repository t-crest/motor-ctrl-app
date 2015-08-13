// Copyright (C) 1991-2011 Altera Corporation
// Your use of Altera Corporation's design tools, logic functions 
// and other software and tools, and its AMPP partner logic 
// functions, and any output files from any of the foregoing 
// (including device programming or simulation files), and any 
// associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License 
// Subscription Agreement, Altera MegaCore Function License 
// Agreement, or other applicable license agreement, including, 
// without limitation, that your use is for the sole purpose of 
// programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the 
// applicable agreement for further details.

`timescale 1ns/1ns

module ssg_emb_pwm (
	input           clk,
	input           reset_n,

    // Avalon-MM in system clk domain
	input               avs_write_n,
	input               avs_read_n,
	input [3:0]         avs_address,
	input [31:0]        avs_writedata,
	output reg [31:0]   avs_readdata,

    input [2:0]         pwm_control,
	
	input				sync_in,
    
	output wire         u_h,
	output wire         u_l,
	output wire         v_h,
	output wire         v_l,
	output wire         w_h,
	output wire         w_l,
	output reg          carrier_latch,
	output reg			encoder_strobe_n,
	output wire         sync_out,
	output wire         start_adc,
	output reg [15:0]   carrier
);											

reg [2:0] gate_h;
reg [2:0] gate_l;
assign u_h = gate_h[0];
assign v_h = gate_h[1];
assign w_h = gate_h[2];
assign u_l = gate_l[0];
assign v_l = gate_l[1];
assign w_l = gate_l[2];
reg start;

wire pwm_enable = pwm_control[0];
wire en_lower = pwm_control[1];
wire en_upper = pwm_control[2];

    //
// Avalon register interface
//
reg [14:0] pwm_reg[0:2];    // 0:U 1:V 2:W
reg [14:0] max_reg;
reg [7:0] block_reg;
reg [14:0] trigger_up_reg;
reg [14:0] trigger_down_reg;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        pwm_reg[0] <= 15'b0;
        pwm_reg[1] <= 15'b0;
        pwm_reg[2] <= 15'b0;
        max_reg <= 15'b0;
        block_reg <= 8'b0;
        trigger_up_reg <= 15'b0;
        trigger_down_reg <= 15'b0;
    end
    else if (~avs_write_n)
    begin
        case (avs_address)
            4'h1:   pwm_reg[0] <= avs_writedata[14:0];
            4'h2:   pwm_reg[1] <= avs_writedata[14:0];
            4'h3:   pwm_reg[2] <= avs_writedata[14:0];
            4'h4:   max_reg <= avs_writedata[14:0];
            4'h5:   block_reg <= avs_writedata[7:0];
            4'h6:   trigger_up_reg <= avs_writedata[14:0];
            4'h7:   trigger_down_reg <= avs_writedata[14:0];
        endcase
    end

always @(*)
    case (avs_address)
        4'h1:   avs_readdata <= {17'b0, pwm_reg[0]};
        4'h2:   avs_readdata <= {17'b0, pwm_reg[1]};
        4'h3:   avs_readdata <= {17'b0, pwm_reg[2]};
        4'h4:   avs_readdata <= {17'b0, max_reg};
        4'h5:   avs_readdata <= {24'b0, block_reg};
        4'h6:   avs_readdata <= {17'b0, trigger_up_reg};
        4'h7:   avs_readdata <= {17'b0, trigger_down_reg};
        4'h8:   avs_readdata <= {22'b0, gate_l[0], gate_h[0], gate_l[1], gate_h[1], gate_l[2], gate_h[2]};
        4'h9:   avs_readdata <= {16'b0, carrier};
        default:    avs_readdata <= 32'h0;
    endcase

// Create local copies of control regs in case they are updated mid-cycle
reg [14:0] pwm[0:2];
generate
    genvar j;
    for (j = 0; j < 3; j = j + 1)
    begin : reg_gen_loop
        always @(posedge clk or negedge reset_n)
            if (~reset_n)
                pwm[j] <= 15'b0;
            else if (~pwm_enable)
                pwm[j] <= 15'b0;
            else if (carrier_latch)
                pwm[j] <= pwm_reg[j];
    end
endgenerate

    // The counter counts alternately up and down between zero and max_reg value
// The count value is compared against the other registers to determine when
// to drive the various outputs
reg [14:0] counter;    
reg [14:0] counter_nxt;    
reg up;
reg sync;
always @(*)
    if (up)
        counter_nxt <= counter + 15'b1;
    else
        counter_nxt <= counter - 15'b1;
        
always @(posedge clk or negedge reset_n)
begin
    if (~reset_n)
    begin
        counter <= 15'b0;
        up <= 1'b1;
        carrier_latch <= 1'b0;
    end
	else
	begin
		if (~pwm_enable | sync_in)
		begin
			counter <= 15'b0;
			up <= 1'b1;
			start <= 1'b0;
		end
		else
		begin
			counter <= counter_nxt;
				
			if (up & (counter_nxt == trigger_up_reg) | ~up & (counter_nxt == trigger_down_reg))
				// Set start pulse for ADC
				start <= 1'b1;
			else
				start <= 1'b0;
		end
		if (up & (counter_nxt == max_reg) | ~up & (counter_nxt == 15'b0))
		begin
			// change direction and set latch enable
			up <= ~up;
			carrier_latch <= 1'b1;
		end
		else
			carrier_latch <= 1'b0;
    end
end

	always @(*)
        if (~up & (counter_nxt == 15'b0))
            // synchronise all PWM modules
            sync <= 1'b1;
        else
            sync <= 1'b0;

assign sync_out = sync | sync_in;
assign start_adc = start;

// carrier ramps up during one up/down cycle of the counter then resets
always @(posedge clk or negedge reset_n)
    if (~reset_n)
        carrier <= 15'b0;
    else if (~pwm_enable | sync_in | ~up & (counter_nxt == 15'b0))
        carrier <= 15'b0;
    else
        carrier <= carrier + 16'b1;

reg [2:0] strobe_count;
always @(posedge clk or negedge reset_n)
	if (~reset_n)
		strobe_count <= 3'b000;
	else if (carrier_latch)
		strobe_count <= 3'b001;
	else if (|strobe_count)
		strobe_count <= strobe_count + 3'b001;

always @(posedge clk or negedge reset_n)
	if (~reset_n)
		encoder_strobe_n <= 1'b1;
	else if (carrier_latch)
		encoder_strobe_n <= 1'b0;
	else if (|strobe_count)
		encoder_strobe_n <= 1'b0;
	else
		encoder_strobe_n <= 1'b1;
		
// Generate gate drives from counter state, inserting required dead time
// between H & L driver transitions
// generate index corresponds to phases: 0:U 1:V 2:W
reg [7:0] dead_count[0:2];
reg compare[0:2];
reg compare_r[0:2];
generate
    genvar i;
        for (i = 0; i < 3; i = i + 1)
        begin : op_gen_loop
            always @(posedge clk or negedge reset_n)
                if (~reset_n)
                begin
                    dead_count[i] <= 8'b0;
                    compare[i] <= 1'b0;
                    compare_r[i] <= 1'b0;
                    gate_h[i] <= 1'b0;
                    gate_l[i] <= 1'b0;
                end
                else if (~pwm_enable)
                begin
                    dead_count[i] <= block_reg;
                    compare[i] <= 1'b0;
                    compare_r[i] <= 1'b0;
                    gate_h[i] <= 1'b0;
                    gate_l[i] <= 1'b0;
                end
                else
                begin
                    compare_r[i] <= compare[i];
                    if (pwm[i] > counter)
                        compare[i] <= 1'b1;
                    else
                        compare[i] <= 1'b0;
                    if (compare[i] != compare_r[i])
                    begin
                        dead_count[i] <= block_reg;
                        gate_h[i] <= 1'b0;
                        gate_l[i] <= 1'b0;
                    end
                    else if (dead_count[i] > 8'b0)
                    begin
                        dead_count[i] <= dead_count[i] - 8'b1;
                        gate_h[i] <= 1'b0;
                        gate_l[i] <= 1'b0;
                    end
                    else
                    begin
                        gate_h[i] <= compare[i] & en_upper;	
                        gate_l[i] <= ~compare[i] & en_lower;
                    end
                end
        end
endgenerate
                
endmodule   // ssg_emb_pwm
