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

module ssg_emb_sd_adc_diff (
    input   clk,                    // System clock
    input   clk_adc,                // ADC clock
    input   reset_n,                // System reset
    input   data,                   // ADC data in clk_adc domain
    input   start,                  // Start conversion pulse in system clk domain
    input   dec_rate,               // Decimation rate control register bit in clk_adc domain 1: M=64 or 8, 0: M=128 or 16
    
    // All outputs in clk_adc domain
    output wire [21:0]  cn_out,     // Third stage integrator output
    output reg          cnr16,      // Decimator pulse for low res conversion
    output reg          cnr128,     // Decimator pulse for highres conversion
    output wire         latch_en    // Output sample latch enable
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
    end
    else
    begin
        cn1 <= cn1 + delta1;
        cn2 <= cn2 + cn1;
    end

assign cn_out = cn2;

// Synchronise start pulse to clk_adc by strecthing it and handshaking between
// the clock domains
reg start_r;
reg start_adc_sync;
reg start_adc_sync2;
reg start_sync_adc;
reg start_sync2_adc;
wire start_adc;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        start_r <= 1'b0;
        start_adc_sync <= 1'b0;
        start_adc_sync2 <= 1'b0;
    end
    else
    begin
        if (start)
            start_r <= 1'b1;
        else if (start_adc_sync2)
            start_r <= 1'b0;
        start_adc_sync <= start_sync2_adc;
        start_adc_sync2 <= start_adc_sync;
    end
        
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
    begin
        start_sync_adc <= 1'b0;
        start_sync2_adc <= 1'b0;
    end
    else
    begin
        start_sync_adc <= start_r;
        start_sync2_adc <= start_sync_adc;
    end

assign start_adc = ~start_sync2_adc & start_sync_adc;

// Generate sample strobes for high resolution decimator and output result latch
reg [8:0] counter;
reg run;
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
    begin
        counter <= 9'b0;
        run <= 1'b0;
    end
    else
    begin
        if (start_adc)
            run <= 1'b1;
        else if (counter == 9'd399)
        begin
            run <= 1'b0;
            counter <= 9'b0;
        end
        else if (run)
            counter <= counter + 9'b1;
    end

assign latch_en = run & (counter == 9'd399);

// Generate pulses to tell high resolution decimator when to take a sample
// Use start_adc to force first result to be used
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
        cnr128 <= 1'b0;
    else
    begin
        if (dec_rate)
            // for high res converter M = 64
            cnr128 <= run & (counter[5:0] == 6'b111111) | start_adc;
        else
            // for high res converter M = 128
            cnr128 <= run & (counter[6:0] == 7'b1111111) | start_adc;
    end

// For low resolution decimator use a free running counter
reg [3:0] counter16;
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
        counter16 <= 4'b0;
    else
        counter16 <= counter16 + 4'b1;

always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
        cnr16 <= 1'b0;
    else
    begin
        if (dec_rate)
            // for low res converter M = 8
            cnr16 <= (counter16[2:0] == 3'b111);
        else
            // for low res converter M = 16
            cnr16 <= (counter16[3:0] == 4'b1111);
    end

endmodule   // ssg_emb_sd_adc_diff