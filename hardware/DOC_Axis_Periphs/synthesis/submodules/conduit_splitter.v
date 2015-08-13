// (C) 2001-2014 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


`timescale 1 ns / 1 ns

(* altera_attribute = "-name IP_TOOL_NAME conduit_splitter; -name IP_TOOL_VERSION 13.0" *)

module conduit_splitter
#(
    parameter OUTPUT_NUM = 2
)
(
    input conduit_input,
    output conduit_output_0,
    output conduit_output_1,
    output conduit_output_2,
    output conduit_output_3,
    output conduit_output_4
);

generate 
    assign conduit_output_0 = conduit_input;
    assign conduit_output_1 = conduit_input;
    assign conduit_output_2 = conduit_input;
    assign conduit_output_3 = conduit_input;
    assign conduit_output_4 = conduit_input;
endgenerate
endmodule
