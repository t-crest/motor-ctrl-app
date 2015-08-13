/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2014 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
* Altera does not recommend, suggest or require that this reference design    *
* file be used in conjunction or combination with any other product.          *
******************************************************************************/

#include "includes.h"

/**
 * @file endat_api.c
 *
 * @brief Low level EnDat encoder interface
 */

/*!
 * \addtogroup COMPONENTS
 *
 * @{
 */

/*!
 * \addtogroup ENDAT EnDat Encoder interface
 *
 * @{
 */

//
// Reset the EnDat interface
//
void EnDat_Reset(unsigned int base_addr) {
	IOWR_32DIRECT(base_addr, ENDAT_KONF1_ADR*4, ENDAT_CFG1_RESET);
}

//
// Reset the encoder and wait 50ms as per the EnDat spec
//
void EnDat_Enc_Reset(unsigned int base_addr) {
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_ENC_RST, 0, 0);
	OSTimeDlyHMSM(0, 0, 0, 50);
}

//
// Low level write fiunction for EnDat interface
//
/*void EnDat_Write(unsigned int base_addr, unsigned int reg, unsigned int data) {
	IOWR_32DIRECT(base_addr, reg*4, data);
}*/

//
// Low level read fiunction for EnDat interface
//
/*unsigned int EnDat_Read(unsigned int base_addr, unsigned int reg) {
	return IORD_32DIRECT(base_addr, reg*4);
}*/

//
// Write a parameter to the Endat encoder. Wait for access time and reset status register.
//
void EnDat_Enc_Parameter_Write(unsigned int base_addr, int mrs, int adr, int data) {
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_SELECT, mrs, 0);
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_ENC_RX, adr, data);
	OSTimeDlyHMSM(0, 0, 0, 15);
	IOWR_32DIRECT(base_addr, ENDAT_STATUS_ADR*4, 0xffffffff);
}

//
// Read a parameter from the Endat encoder. Wait for access time and mas returned data
// to a 16-bit result. Reset status register.
//
void EnDat_Enc_Parameter_Read(unsigned int base_addr, int mrs, int adr, int *data) {
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_SELECT, mrs, 0);
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_ENC_TX, adr, 0);
	OSTimeDlyHMSM(0, 0, 0, 15);
	*data = IORD_32DIRECT(base_addr, ENDAT_EMPF1_ADR*4) & 0xffff;
	IOWR_32DIRECT(base_addr, ENDAT_STATUS_ADR*4, 0xffffffff);
}

//
// Send a mode command to the encoder
//
void EnDat_Enc_Send_Mode_Cmd(unsigned int base_addr, unsigned int mode, unsigned int mrs, unsigned int para) {
	IOWR_32DIRECT(base_addr, ENDAT_SENDE_ADR*4, ((mode & 0x3f)<<24) | ((mrs & 0xff)<<16) | (para & 0xffff));
	OSTimeDlyHMSM(0, 0, 0, 15);
	IOWR_32DIRECT(base_addr, ENDAT_STROBE_ADR*4, 0x11111111);
	OSTimeDlyHMSM(0, 0, 0, 15);
	IOWR_32DIRECT(base_addr, ENDAT_STATUS_ADR*4, 0xffffffff);
}

//
// Read data width form EnDat encoder parameters
//
int EnDat_Enc_Read_Sensor_Data_Width(unsigned int base_addr) {
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_SELECT, 0xA1, 0);
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_ENC_TX, 0x0D, 0);
	return IORD_32DIRECT(base_addr, ENDAT_EMPF1_ADR*4) & 0x3f;
}

//
// Read warnings from sensor
//
int EnDat_Enc_Rd_Sensor_Warning_Data(unsigned int base_addr) {
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_SELECT, 0xB9, 0);
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_ENC_TX, 1, 0);
	return IORD_32DIRECT(base_addr, ENDAT_EMPF1_ADR*4) & 0x1f;
}

//
// Read errors from sensor
//
int EnDat_Enc_Rd_Sensor_Error_Data(unsigned int base_addr) {
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_SELECT, 0xB9, 0);
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_ENC_TX, 0, 0);
	return IORD_32DIRECT(base_addr, ENDAT_EMPF1_ADR*4) & 0x7f;
}

//
// Clear errors from sensor
//
void EnDat_Enc_Clear_Sensor_Errors(unsigned int base_addr) {
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_SELECT, 0xB9, 0);
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_ENC_RX, 0, 0);
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_ENC_RST, 0, 0);
}

//
// Clear warnings from sensor
//
void EnDat_Enc_Clear_Sensor_Warnings(unsigned int base_addr) {
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_SELECT, 0xB9, 0);
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_ENC_RX, 1, 0);
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_ENC_RST, 0, 0);
}

//
// Tell the encoder to send position data
//
void EnDat_Enc_Tx_Pos(unsigned int base_addr) {
	EnDat_Enc_Send_Mode_Cmd(base_addr, ENDAT_TX_POS, 0, 0);
}

/*!
 * @}
 */

/*!
 * @}
 */
