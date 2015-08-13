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
//
// For a 3 stage Sinc3 filter, each 2x increase in decimation rate
// requires 3 extra bits in the data path so for a decimation rate
// of 128 we need 21 bits (3x7) plus one sign bit

`timescale 1ns/1ns

module ssg_emb_dc_ballast_diff (
    input   clk_adc,                // ADC clock
    input   reset_n,                // System reset
    input   data,                   // ADC data in clk_adc domain
    input   dec_rate,               // Decimation rate control register bit in clk_adc domain 1: M=64 or 8, 0: M=128 or 16
    
    // All outputs in clk_adc domain
    output reg [21:0]   cn_out,     // Third stage integrator output
    output reg          cnr128      // Decimator pulse for highres conversion
);

reg [21:0] delta1, cn1, cn2;

always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
        delta1 <= 22'b0;
    else if (data)
        delta1 <= delta1 + 22'b1;

always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
    begin
        cn1 <= 22'b0;
        cn2 <= 22'b0;
        cn_out <= 22'b0;
    end
    else
    begin
        cn1 <= cn1 + delta1;
        cn2 <= cn2 + cn1;
        cn_out <= cn2;
    end


// Generate sample strobes for high resolution decimator and output result latch
reg [8:0] counter;
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
        counter <= 9'b0;
    else
        counter <= counter + 9'b1;

// Generate pulses to tell high resolution decimator when to take a sample
// Use start_adc to force first result to be used
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
        cnr128 <= 1'b0;
    else
    begin
        if (dec_rate)
            // for high res converter M = 64
            cnr128 <= (counter[5:0] == 6'b111111);
        else
            // for high res converter M = 128
            cnr128 <= (counter[6:0] == 7'b1111111);
    end

endmodule   // ssg_emb_dc_ballast_diff