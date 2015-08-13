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

// Sinc3 filter differentiator based on "Combining the ADS1202 with
// an FPGA Digital Filter for Current Measurement in Motor Control
// Applications" TI Application report SBAA094 June 2003

// Sinc3 filter differentiator based on "Combining the ADS1202 with
// an FPGA Digital Filter for Current Measurement in Motor Control
// Applications" TI Application report SBAA094 June 2003
//
// For a 3 stage Sinc3 filter, each 2x increase in decimation rate
// requires 3 extra bits in the data path

`timescale 1ns/1ns

module ssg_emb_dc_ballast_dec128 (
    input               clk,        // ADC clock
    input               reset_n,    // syetm reset
    input               cnr128,     // Decimator pulse
    input               dec_rate,   // Decimation rate control register bit 1: M=64, 0: M=128
    input [15:0]        offset,     // zero offset register
    input [21:0]        cn_in,      // Input from integrator
    
    output reg [15:0]   sample      // Output sample
);

reg [18:0] dn0, dn1, dn3, dn5;
reg [19:0] sample_x;
wire [18:0] cn3, cn4, cn5;

always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        dn0 <= 19'b0;
        dn1 <= 19'b0;
        dn3 <= 19'b0;
        dn5 <= 19'b0;
        sample_x <= 20'b0;
    end
    else if (cnr128)
    begin
        dn0 <= dec_rate ? cn_in[18:0] : cn_in[21:3];
        dn1 <= dn0;
        dn3 <= cn3;
        dn5 <= cn4;
    end
    else
        sample_x <= {cn5[18], cn5} - {offset[15], offset, 3'b000};

assign cn3 = dn0 - dn1;
assign cn4 = cn3 - dn3;
assign cn5 = cn4 - dn5;

always @(posedge clk or negedge reset_n)
    if (~reset_n)
        sample <= 16'b0;
    else
        if (sample_x[19:18] == 2'b00)
            sample <= sample_x[18:3];
        else if (sample_x[18] == 1'b0)      // Limit to +ve full scale
            sample <= 16'h7FFF;
        else if (sample_x[18] == 1'b1)      // Limit to 0
            sample <= 16'h0000;

endmodule   // ssg_emb_dc_ballast_dec128
