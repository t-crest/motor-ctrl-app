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

// 1.2	Move overcurent detection to adc clock domain to avoid clock crosing problems
//		Add capture registers for overcurrent detection
// 1.3  Synchronise control reg bits
//      Tidy up some synthesis warnings
// 1.4 Use shorter counter value when decimation rate is set to lower value so that
//     conversion time is reduced

`timescale 1ns/1ns

module ssg_emb_sd_adc (
	input           clk,
	input           clk_adc,
	input           reset_n,

    // Avalon-MM in system clk domain
	input               avs_write_n,
	input               avs_read_n,
	input [3:0]         avs_address,
	input [31:0]        avs_writedata,
	output reg [31:0]   avs_readdata,
	output wire         avs_irq,

    // ADC inputs in clk_adc domain
	input           sync_dat_u,
	input           sync_dat_w,
    
    // Start conversion pulse in system clk domain
	input           start,              // was cnr128_reset
    
    // status output in system clk domain
	output reg      overcurrent
);											

//
// Avalon register interface
//
// Registers are written in clk domain but used in clk_adc domain.
// Control bits are explicitly synchronised.
// Other registers are assumed to be setup by software in advance of the PWM/ADC being enabled
// and any conversions being made. Values will be stabe and there is no need for any further
// synchronisation. timing constraints will be used to flag these false paths.
//
reg [15:0] offset_u_reg;
reg [15:0] offset_w_reg;
reg [9:0] i_peak_reg;
reg [2:0] control_reg;
reg [2:0] control_reg_s1;
reg [2:0] control_reg_s2;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        offset_u_reg <= 16'b0;
        offset_w_reg <= 16'b0;
        i_peak_reg <= 10'b0;
        control_reg <= 3'b0;
    end
    else if (~avs_write_n)
    begin
        case (avs_address)
            4'h1:   offset_u_reg <= avs_writedata[15:0];
            4'h2:   offset_w_reg <= avs_writedata[15:0];
            4'h3:   i_peak_reg <= avs_writedata[9:0];
            4'h4:   control_reg <= avs_writedata[2:0];
        endcase
    end
	
//
// Synch control reg to clk_adc
// 
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
    begin
		control_reg_s1 <= 3'b0;
		control_reg_s2 <= 3'b0;
	end
	else
	begin
		control_reg_s1 <= control_reg;
		control_reg_s2 <= control_reg_s1;
	end
wire dec_rate = control_reg_s2[2];         // Decimation rate
wire overcurrent_error = control_reg_s2[1];        // 
wire overvolt_en = control_reg_s2[0];

wire irq_ack = (avs_address == 4'h5) & ~avs_write_n & avs_writedata[0];

reg conv_done_u;
reg conv_done_w;
reg overcurrent_u;
reg overcurrent_w;
reg [15:0] sample128_w_r;
reg [15:0] sample128_u_r;
reg [9:0] oc_capture_u;
reg [9:0] oc_capture_w;
always @(*)
    case (avs_address)
        4'h1:   avs_readdata <= {16'b0, offset_u_reg};
        4'h2:   avs_readdata <= {16'b0, offset_w_reg};
        4'h3:   avs_readdata <= {12'b0, i_peak_reg};
        4'h4:   avs_readdata <= {29'b0, control_reg};
        4'h6:   avs_readdata <= {27'b0, conv_done_u,
                                        conv_done_w, overcurrent_u, overcurrent_w, overcurrent};
        4'h7:   avs_readdata <= {16'b0, sample128_u_r};
        4'h8:   avs_readdata <= {16'b0, sample128_w_r};
        4'h9:   avs_readdata <= {12'b0, i_peak_reg};
		
		4'hc:	avs_readdata <= {12'b0, oc_capture_u};
		4'hd:	avs_readdata <= {12'b0, oc_capture_w};
        default:    avs_readdata <= 32'h0;
    endcase

// Synchronise latch signals to system clock domain
wire latch_u;
wire latch_w;
reg latch_u_sync;
reg latch_w_sync;
reg latch_u_sync2;
reg latch_w_sync2;
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        latch_u_sync <= 1'b0;
        latch_w_sync <= 1'b0;
        latch_u_sync2 <= 1'b0;
        latch_w_sync2 <= 1'b0;
    end
    else
    begin
        latch_u_sync <= latch_u;
        latch_w_sync <= latch_w;
        latch_u_sync2 <= latch_u_sync;
        latch_w_sync2 <= latch_w_sync;
    end

// Conversion complete interrupt
always @(posedge clk or negedge reset_n)
    if (~reset_n)
    begin
        conv_done_u <= 1'b0;
        conv_done_w <= 1'b0;
    end
    else if (irq_ack)
    begin
        conv_done_u <= 1'b0;
        conv_done_w <= 1'b0;
    end
    else
    begin
        if (latch_u_sync2) conv_done_u <= 1'b1;
        if (latch_w_sync2) conv_done_w <= 1'b1;
    end

assign avs_irq = conv_done_u & conv_done_w;

// Sinc3 filter differentiators
//
// *** NOTE
// Input bit stream is inverted to match FalconEye implementation
wire [21:0] cn_out_u;
wire cnr16_u;
wire cnr128_u;
ssg_emb_sd_adc_diff diff_u (
    .clk                (clk),
    .clk_adc            (clk_adc),
    .reset_n            (reset_n),
    .data               (sync_dat_u),
    .start              (start),
    .dec_rate           (dec_rate),
    
    .cn_out             (cn_out_u),
    .cnr16              (cnr16_u),
    .cnr128             (cnr128_u),
    .latch_en           (latch_u)
);

wire [21:0] cn_out_w;
wire cnr16_w;
wire cnr128_w;
ssg_emb_sd_adc_diff diff_w (
    .clk                (clk),
    .clk_adc            (clk_adc),
    .reset_n            (reset_n),
    .data               (sync_dat_w),
    .start              (start),
    .dec_rate           (dec_rate),
    
    .cn_out             (cn_out_w),
    .cnr16              (cnr16_w),
    .cnr128             (cnr128_w),
    .latch_en           (latch_w)
);

// Sinc3 filter decimators for high resolution samples
wire [15:0] sample128_w;
wire [15:0] sample128_u;
ssg_emb_sd_adc_dec128 decimator128_u (
    .clk                (clk_adc),
    .reset_n            (reset_n),
    .cnr128             (cnr128_u),
    .dec_rate           (dec_rate),
    .offset             (offset_u_reg),
    .cn_in              (cn_out_u),
    
    .sample             (sample128_u)
);

ssg_emb_sd_adc_dec128 decimator128_w (
    .clk                (clk_adc),
    .reset_n            (reset_n),
    .cnr128             (cnr128_w),
    .dec_rate           (dec_rate),
    .offset             (offset_w_reg),
    .cn_in              (cn_out_w),
    
    .sample             (sample128_w)
);

// Register samples
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
    begin
        sample128_u_r <= 16'b0;
        sample128_w_r <= 16'b0;
    end
    else
    begin
        sample128_u_r <= sample128_u;
        sample128_w_r <= sample128_w;
    end

// Sinc3 filter decimators for low resolution samples for current fail
wire [9:0] sample16_u;
ssg_emb_sd_adc_dec16 decimator16_u (
    .clk                (clk_adc),
    .reset_n            (reset_n),
    .cnr16              (cnr16_u),
    .dec_rate           (dec_rate),
    .cn_in              (cn_out_u),
    
    .sample             (sample16_u)
);

wire [9:0] sample16_w;
ssg_emb_sd_adc_dec16 decimator16_w (
    .clk                (clk_adc),
    .reset_n            (reset_n),
    .cnr16              (cnr16_w),
    .dec_rate           (dec_rate),
    .cn_in              (cn_out_w),
    
    .sample             (sample16_w)
);

// Absolute value of current measurements
reg [9:0] sample16_u_abs;
reg [9:0] sample16_w_abs;
always @(*)
    if (sample16_u[9])
        sample16_u_abs <= (~sample16_u + 10'b1);
    else
        sample16_u_abs <= sample16_u;

always @(*)
    if (sample16_w[9])
        sample16_w_abs <= (~sample16_w + 10'b1);
    else
        sample16_w_abs <= sample16_w;

// Current fail logic
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
	begin
        overcurrent_u <= 1'b0;
        overcurrent_w <= 1'b0;
    end
    else if (overcurrent_error | ~overvolt_en)
    begin
        overcurrent_u <= 1'b0;
        overcurrent_w <= 1'b0;
    end
    else
    begin
        // Change from FalconEye design to allow parallel detection in both channels
        if (sample16_u_abs >= i_peak_reg)
            overcurrent_u <= 1'b1;
        else
            overcurrent_u <= 1'b0;
        if (sample16_w_abs >= i_peak_reg)
            overcurrent_w <= 1'b1;
        else
            overcurrent_w <= 1'b0;
    end

// Capture absolute value on each rising edge of overcurrent flag
// By the time software reads them , they will be stable in clk domian
reg overcurrent_u_r, overcurrent_w_r;
always @(posedge clk_adc or negedge reset_n)
    if (~reset_n)
	begin
        overcurrent_u_r <= 1'b0;
        overcurrent_w_r <= 1'b0;
    end
    else
    begin
		overcurrent_u_r <= overcurrent_u;
		overcurrent_w_r <= overcurrent_w;
		
        if (overcurrent_u & ~overcurrent_u_r)
		begin
			oc_capture_u <= sample16_u_abs;
		end
        if (overcurrent_w & ~overcurrent_w_r)
		begin
			oc_capture_w <= sample16_w_abs;
		end
    end

wire overcurrent_x = overcurrent_u | overcurrent_w;

// Double sample to synch to clk domain
reg overcurrent_xx;
always @(posedge clk or negedge reset_n)
	if (~reset_n)
	begin
		overcurrent_xx <= 1'b0;
		overcurrent <= 1'b0;
	end
	else
	begin
		overcurrent_xx <= overcurrent_x;
		overcurrent <= overcurrent_xx;
	end

endmodule   // ssg_emb_sd_adc
