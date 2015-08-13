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

// 1.2 extra IDLE state to DSM which can be used to disable the PWM 
//     under software control 
//     Correct operation of state machine so that (e1 | e3) term does not mask 
//     remainder of state machine and prevent pwm being disable under fault condition.

`timescale 1ns/1ns

module ssg_emb_dsm (
	input           clk,
	input           reset_n,

    // Avalon-MM in system clk domain
	input               avs_write_n,
	input               avs_read_n,
	input               avs_address,
	input [31:0]        avs_writedata,
	output reg [31:0]   avs_readdata,

	input               overcurrent,
	input               overvoltage,
	input               undervoltage,
	input               chopper,
	input               dc_link_clk_err,
    input               igbt_err,
    
    output wire     error_out,
    output wire     overcurrent_latch,
    output wire     overvoltage_latch,
    output wire     undervoltage_latch,
    output wire     dc_link_clk_err_latch,
    output wire     igbt_err_latch,
    output reg      chopper_latch,
    output reg [2:0] pwm_control
);											

//
// Avalon register interface
//
reg [2:0] control_reg;
reg clear_status;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        control_reg <= 3'b0;
        clear_status <= 1'b0;
    end
    else if (~avs_write_n)
    begin
        case (avs_address)
            4'h0:   control_reg <= avs_writedata[2:0];
            4'h1:   clear_status <= 1'b1;
        endcase
    end
    else
        clear_status <= 1'b0;

reg [2:0] state;
reg [4:0] status_bits;
always @(*)
    case (avs_address)
        4'h0:   avs_readdata <= {29'b0, control_reg};
        4'h1:   avs_readdata <= {20'b0, state, pwm_control, 1'b1, status_bits};
        default:    avs_readdata <= 32'h0;
    endcase

// Status bits are sticky and cleared by write 1 to the bit(s) in status reg
reg [4:0] avs_writedata_r;
reg e1, e2;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
        avs_writedata_r <= 5'b0;
    else
        avs_writedata_r <= avs_writedata[4:0];
        
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        status_bits[3:0] <= 4'b0;
        chopper_latch <= 1'b0;
        e1 <= 1'b0;
        e2 <= 1'b0;
    end
    else if (clear_status)
        status_bits[3:0] <= status_bits[3:0] & ~avs_writedata_r[3:0];
    else
    begin
        chopper_latch <= chopper;
        e1 <= overvoltage_latch | igbt_err_latch | undervoltage_latch; 	
        e2 <= overcurrent_latch | dc_link_clk_err_latch ; 					
        
        status_bits[2:1] <= status_bits[2:1] | {undervoltage, overvoltage};
        if ((state == 2) || (state == 3))
        begin
            status_bits[0] <= status_bits[0] | overcurrent;
            status_bits[3] <= status_bits[3] | dc_link_clk_err;
        end
    end

assign overcurrent_latch = status_bits[0];
assign overvoltage_latch = status_bits[1];
assign undervoltage_latch = status_bits[2];
assign dc_link_clk_err_latch = status_bits[3];
assign igbt_err_latch = status_bits[4];
assign error_out = e1 | e2;

// Accumulate IGBT errors
reg [7:0] igbt_cnt;
reg igbt_err_r;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
		igbt_cnt <= 8'b0;
        igbt_err_r <= 1'b0;
        status_bits[4] <= 1'b0;
    end
    else if (clear_status)
        status_bits[4] <= status_bits[4] & ~avs_writedata_r[4];
	else
    begin
		igbt_err_r <= igbt_err;
		if (igbt_err & ~igbt_err_r)
        begin
			igbt_cnt <= 8'b0;
		end
		if (igbt_err_r)
			igbt_cnt <= igbt_cnt + 8'b1;
		if (igbt_cnt == 8'd75)
			status_bits[4] <=  status_bits[4] | igbt_err ;
	end

parameter   init    = 3'b000,
            prech   = 3'b001,
            prer    = 3'b010,
            run     = 3'b011,
            error   = 3'b100,
			idle    = 3'b101;

parameter   pwm_disable = 3'b000,
            pwm_enable  = 3'b001,
            pwm_lower   = 3'b011,
            pwm_both    = 3'b111;

always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
		state <= 3'b0;
        pwm_control <= pwm_disable;
    end
	else
    begin
		case (state)
			idle:
				if (e1)
					state <= error;
				else
				begin
					// Idle State - disable PWM under software control
					pwm_control <= pwm_disable;
					if (control_reg == init)
						state <= init;					
				end

			init:
				if (e1)
					state <= error;
				else
				begin
					// Init State - enable Drive-PWM
					pwm_control <= pwm_enable;
					if (control_reg == prech)
						state <= prech;
					else if (control_reg == idle)
						state <= idle;							
				end
			
			prech:
				if (e1)
					state <= error;
				else
				begin
					// Pre-Charge State (Bootstrap) - enable Drive-PWM and lower Bridges(EN_Lower)
					pwm_control <= pwm_lower; 
					if (control_reg == prer)
						state <= prer;
					else if (control_reg == init)
						state <= init;
				end
			
			prer:
				begin
					// Pre-Run State (same as Pre-Charge but consider Errors)
					if (e1 | e2)
						state <= error;
					else
					begin
						if (control_reg == run)
							state <= run;
						else if (control_reg == prech)
							state <= prech;
						else if (control_reg == init)
							state <= init;
					end
				end
			
			run:
				begin
					// Run State - enable Drive-PWM and both Bridges (EN_Upper and EN_Lower)
					if (e1 | e2)
						state <= error;
					else
					begin
						pwm_control <= pwm_both;
						if (control_reg == init)
							state <= init;
					end
				end
			
			error:
				begin
					// Error State - disable whole Drive-PWM 
					pwm_control <= pwm_disable;
					if (control_reg == init)
						state <= init;
				end
				
			default:
				begin
					// Fault - change to state Error
					pwm_control <= pwm_disable;
					state <= error;
				end
		endcase
	end







endmodule   // ssg_emb_dsm
