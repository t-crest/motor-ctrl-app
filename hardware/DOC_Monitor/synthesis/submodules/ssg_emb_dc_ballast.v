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

module ssg_emb_dc_ballast (
	input           clk,
	input           clk_adc,
	input           reset_n,

    // Avalon-MM in clk domain
	input               avs_write_n,
	input               avs_read_n,
	input [3:0]         avs_address,
	input [31:0]        avs_writedata,
	output reg [31:0]   avs_readdata,

    // ADC inputs in clk_adc domain
	input           sync_dat,
    
    // Enable input (asynch)
	input           dc_link_enable,
    
    // status outputs in clk domain
	output reg      overvoltage,
	output reg      undervoltage,
    output reg      chopper
);											

//
// Avalon register interface
//
reg [15:0] offset_reg;
reg [15:0] offset_reg_s;
reg [15:0] dec_reg;
reg [15:0] ref_disable_reg;
reg [15:0] link_ref_reg;
reg [15:0] bottom_ref_reg;
reg [10:0] brake_t_reg;
reg [15:0] brake_max_reg;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        offset_reg <= 16'b0;
        dec_reg <= 16'b0;
        ref_disable_reg <= 15'b0;
        link_ref_reg <= 16'b0;
        bottom_ref_reg <= 16'b0;
        brake_t_reg <= 11'b0;
        brake_max_reg <= 16'b0;
    end
    else if (~avs_write_n)
    begin
        case (avs_address)
            4'h1:   offset_reg <= avs_writedata[15:0];
            4'h2:   dec_reg <= avs_writedata[15:0];
            4'h3:   ref_disable_reg <= avs_writedata[15:0];
            4'h4:   link_ref_reg <= avs_writedata[15:0];
            4'h5:   bottom_ref_reg <= avs_writedata[15:0];
            4'h6:   brake_t_reg <= avs_writedata[10:0];
            4'h7:   brake_max_reg <= avs_writedata[15:0];
        endcase
    end

reg [15:0] brake_level;
// Not currently used
always @(*)
	brake_level <= 16'b0;
	
reg [15:0] dc_link;
always @(*)
    case (avs_address)
        4'h1:   avs_readdata <= {16'b0, offset_reg};
        4'h2:   avs_readdata <= {16'b0, dec_reg};
        4'h3:   avs_readdata <= {16'b0, ref_disable_reg};
        4'h4:   avs_readdata <= {16'b0, link_ref_reg};
        4'h5:   avs_readdata <= {16'b0, bottom_ref_reg};
        4'h6:   avs_readdata <= {21'b0, brake_t_reg};
        4'h7:   avs_readdata <= {16'b0, brake_max_reg};
        4'h8:   avs_readdata <= {16'b0, dc_link};
        4'h9:   avs_readdata <= {16'b0, brake_level};
        4'hA:   avs_readdata <= {29'b0, overvoltage, undervoltage, chopper};
        default:    avs_readdata <= 32'h0;
    endcase

// synchronise dec_rate to adc clock domain
reg dec_rate, dec_rate_s;
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
	begin
		dec_rate_s <= 1'b0;
		dec_rate <= 1'b0;
	end
	else
	begin
		dec_rate_s <= dec_reg[0];
		dec_rate <= dec_rate_s;
	end

// Use a toggle to flag writes to offset reg
reg wr_toggle;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
		wr_toggle <= 1'b0;
	else if (~avs_write_n & (avs_address == 4'h1))
		wr_toggle <= ~wr_toggle;

// synch to clk_adc
reg wr_t_s1, wr_t_s2;
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
	begin
		wr_t_s1 <= 1'b0;
		wr_t_s2 <= 1'b0;
	end
	else
	begin
		wr_t_s1 <= wr_toggle;
		wr_t_s2 <= wr_t_s1;
	end

always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
		offset_reg_s <= 16'b0;
	else if (wr_t_s1 ^ wr_t_s2)
		offset_reg_s <= offset_reg;
		
// Sinc3 filter differentiators
wire [21:0] cn_out;
wire cnr128;
ssg_emb_dc_ballast_diff diff (
    .clk_adc            (clk_adc),
    .reset_n            (reset_n),
    .data               (sync_dat),
    .dec_rate           (dec_rate),
    
    .cn_out             (cn_out),
    .cnr128             (cnr128)
);

// Sinc3 filter decimators for high resolution samples
wire [15:0] sample128;
ssg_emb_dc_ballast_dec128 decimator128 (
    .clk                (clk_adc),
    .reset_n            (reset_n),
    .cnr128             (cnr128),
    .dec_rate           (dec_rate),
    .offset             (offset_reg_s),
    .cn_in              (cn_out),
    
    .sample             (sample128)
);

reg p_chopper;
// Not currently used
always @(*)
	p_chopper <= 1'b0;

//
// Delay cnr128 to give time for samples to be updated
//
reg cnr128_d1,  cnr128_d2,  cnr128_d3;
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
    begin
		cnr128_d1 <= 1'b0;
		cnr128_d2 <= 1'b0;
		cnr128_d3 <= 1'b0;
	end
	else
	begin
		cnr128_d1 <= cnr128;
		cnr128_d2 <= cnr128_d1;
		cnr128_d3 <= cnr128_d2;
	end

//
// Synchronise to system clock domain
//
// Assumes clk_adc (e.g. 20MHz) is slower than system clock (e.g 50MHz).
//
reg cnr128_s1, cnr128_s2;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
		cnr128_s1 <= 1'b0;
		cnr128_s2 <= 1'b0;
    end
    else
    begin
		cnr128_s1 <= cnr128_d3;
		cnr128_s2 <= cnr128_s1;
	end
	
// Register samples
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        dc_link <= 16'b0;
    end
    else if (cnr128_s2)
    begin
        dc_link <= sample128;
    end

//
// TODO:
// Chopper is currently disabled as it is believed not to be used in FalconEye
// reference design.
//
// This is now in system clock domain so comparisons with control register values are safe.
//
reg dc_link_enable_s1, dc_link_enable_s2;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
		dc_link_enable_s1 <= 1'b0;
		dc_link_enable_s2 <= 1'b0;
	end
	else
	begin
		dc_link_enable_s1 <= dc_link_enable;
		dc_link_enable_s2 <= dc_link_enable_s1;
	end
	
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        overvoltage <= 1'b0;
        chopper <= 1'b0;
        undervoltage <= 1'b0;
    end
    else
        if  (dc_link_enable_s2 && (dc_link > 16'b0))
        begin
            if (dc_link > ref_disable_reg)
                overvoltage <= 1'b1;
            else 
                overvoltage <= 1'b0;
            if ((dc_link > link_ref_reg) && ~p_chopper)
//                chopper <= 1'b1;
                chopper <= 1'b0;
            else 
                chopper <= 1'b0;
            if (dc_link < bottom_ref_reg)
                undervoltage <= 1'b1;
            else 
                undervoltage <= 1'b0;
        end
        else
        begin
            overvoltage <= 1'b0;
            chopper <= 1'b0;
            undervoltage <= 1'b0;
        end
    
endmodule   // ssg_emb_dc_ballastt
