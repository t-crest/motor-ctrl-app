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

#ifndef ENDAT_API_H_
#define ENDAT_API_H_

/**
 * @file endat_api.h
 *
 * @brief Header file for low level EnDat encoder interface
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

/*****************************************************************************************/
/* address space of EnDat22                                                              */
/*****************************************************************************************/

#define ENDAT_SENDE_ADR      0x00
#define ENDAT_EMPF1_ADR      0x01
#define ENDAT_EMPF1H_ADR     0x02
#define ENDAT_EMPF2_ADR      0x03
#define ENDAT_EMPF3_ADR      0x04
#define ENDAT_KONF1_ADR      0x05
#define ENDAT_KONF2_ADR      0x06
#define ENDAT_KONF3_ADR      0x07
#define ENDAT_STATUS_ADR     0x08
#define ENDAT_S_INT_EN_ADR   0x09
#define ENDAT_TEST1_ADR      0x0a
#define ENDAT_TEST2_ADR      0x0b
#define ENDAT_EMPF4_ADR      0x0c
#define ENDAT_EMPF4H_ADR     0x0d
#define ENDAT_STROBE_ADR     0x0e
#define ENDAT_ID_ADR         0x0f

#define ENDAT_DS1_ADR        0x10
#define ENDAT_DS2_ADR        0x11
#define ENDAT_DS3_ADR        0x12
#define ENDAT_KONF4_ADR      0x13
#define ENDAT_KONF5_ADR      0x14
#define ENDAT_NPV_ADR        0x15
#define ENDAT_NPV_H_ADR      0x16
#define ENDAT_TEST3_ADR      0x17
#define ENDAT_OFFSET2_ADR    0x18
#define ENDAT_OFFSET2_H_ADR  0x19
#define ENDAT_FEHLER_ADR     0x1a
#define ENDAT_F_INT_EN_ADR   0x1b
#define ENDAT_SRM_ADR        0x1c
#define ENDAT_SRM_H_ADR      0x1d
#define ENDAT_NSRPOS1_ADR    0x1e
#define ENDAT_NSRPOS2_ADR    0x1f

#define ENDAT_TEST4_ADR      0x20
#define ENDAT_TEST4_H_ADR    0x21
#define ENDAT_EMPF1S_ADR     0x22
#define ENDAT_EMPF1S_H_ADR   0x23
#define ENDAT_EMPF3S_ADR     0x24
#define ENDAT_DS4_ADR        0x25

// Send register macros
#define ENDAT_TX_POS         0x07
#define ENDAT_TX_POS_ADD     0x38
#define ENDAT_TX_SELECT      0x0E
#define ENDAT_TX_ENC_RX      0x1C
#define ENDAT_TX_ENC_TX      0x23
#define ENDAT_TX_ENC_RST     0x2A
#define ENDAT_TX_ENC_TX_TEST 0x15
#define ENDAT_TX_ENC_RX_COM  0x12
#define ENDAT_TX_ENC_RX_TEST 0x31
#define ENDAT_TX_POS_ADD_MEM 0x09
#define ENDAT_TX_POS_ADD_RX  0x1B
#define ENDAT_TX_POS_PARA    0x24
#define ENDAT_TX_POS_RX_RST  0x2D
#define ENDAT_TX_POS_RX_TEST 0x36

// CFG1 bit macros
#define ENDAT_CFG1_ENDAT    0x80000000
#define ENDAT_CFG1_SSI      0x40000000
#define ENDAT_CFG1_RESET    0x20000000
#define ENDAT_CFG1_FSYS_100 0x18000000
#define ENDAT_CFG1_FSYS_50  0x14000000
#define ENDAT_CFG1_FSYS_32  0x10000000
#define ENDAT_CFG1_FSYS_48  0x08000000
#define ENDAT_CFG1_FSYS_64  0x00000000
#define ENDAT_CFG1_COMP     0x01000000
#define ENDAT_CFG1_AUTOREST_ON  0x00008000
#define ENDAT_CFG1_AUTOREST_OFF 0x00000000
#define ENDAT_CFG1_RESWIN_ON    0x00004000
#define ENDAT_CFG1_RESWIN_OFF   0x00000000
#define ENDAT_CFG1_LENGTH(l)    (l&0x3f)<<8
#define ENDAT_CFG1_FTCLK(s)     (s&0xf)<<4
#define ENDAT_CFG1_DT_ON        0x00000004
#define ENDAT_CFG1_DT_OFF       0x00000000
#define ENDAT_CFG1_DU_ON        0x00000002
#define ENDAT_CFG1_DU_OFF       0x00000000
#define ENDAT_CFG1_HWSTR_ON     0x00000001
#define ENDAT_CFG1_HWSTR_OFF    0x00000000

// CFG2 bit macros
#define ENDAT_CFG2_HWSDEL_OFF 0x00000000
#define ENDAT_CFG2_HWSDEL(d)  (d&0xff)<<24
#define ENDAT_CFG2_FILTER_OFF 0x00000000
#define ENDAT_CFG2_FILTER_3   0x00080000
#define ENDAT_CFG2_FILTER_4   0x00100000
#define ENDAT_CFG2_FILTER_5   0x00180000
#define ENDAT_CFG2_FILTER_6   0x00200000
#define ENDAT_CFG2_FILTER_10  0x00280000
#define ENDAT_CFG2_FILTER_20  0x00300000
#define ENDAT_CFG2_FILTER_40  0x00380000
#define ENDAT_CFG2_TST_HALF_TCLK 0x00000000
#define ENDAT_CFG2_TST_HALF   0x00010000
#define ENDAT_CFG2_TST_1      0x00020000
#define ENDAT_CFG2_TST_1_5    0x00030000
#define ENDAT_CFG2_TST_2      0x00040000
#define ENDAT_CFG2_TST_4      0x00050000
#define ENDAT_CFG2_TST_8      0x00060000
#define ENDAT_CFG2_TST_10     0x00070000
#define ENDAT_CFG2_WDOG_OFF   0x00000000
#define ENDAT_CFG2_WDOG(d)    (d&0xff)<<8
#define ENDAT_CFG2_TIMER_OFF  0x00000000
#define ENDAT_CFG2_TIMER(t)   (t&0xff)

// CFG3 bit macros
#define ENDAT_CFG3_SPEED32    0x00008000
#define ENDAT_CFG3_SPEED64    0x00000000
#define ENDAT_CFG3_DW_ON      0x00000100
#define ENDAT_CFG3_DW_OFF     0x00000000
#define ENDAT_CFG3_STRES(s)   (s&0xf)<<3
#define ENDAT_CFG3_BINARY     0x00000004
#define ENDAT_CFG3_GRAY       0x00000000
#define ENDAT_CFG3_FORMAT_SR  0x00000002
#define ENDAT_CFG3_FORMAT_FT  0x00000000
#define ENDAT_CFG3_PARITY_ON  0x00000001
#define ENDAT_CFG3_PARITY_OFF 0x00000000

// Function prototypes
void EnDat_Reset(unsigned int base_addr);
void EnDat_Enc_Reset(unsigned int base_addr);
//void EnDat_Write(unsigned int base_addr, unsigned int reg, unsigned int data);
//unsigned int EnDat_Read(unsigned int base_addr, unsigned int reg);
void EnDat_Enc_Parameter_Read(unsigned int base_addr, int mrs, int adr, int *data);
void EnDat_Enc_Parameter_Write(unsigned int base_addr, int mrs, int adr, int data);
void EnDat_Enc_Send_Mode_Cmd(unsigned int base_addr, unsigned int mode, unsigned int mrs, unsigned int para);
int EnDat_Enc_Read_Sensor_Data_Width(unsigned int base_addr);
int EnDat_Enc_Rd_Sensor_Warning_Data(unsigned int base_addr);
int EnDat_Enc_Rd_Sensor_Error_Data(unsigned int base_addr);
void EnDat_Enc_Clear_Sensor_Errors(unsigned int base_addr);
void EnDat_Enc_Clear_Sensor_Warnings(unsigned int base_addr);
void EnDat_Enc_Tx_Pos(unsigned int base_addr);

//
// Low level write function for EnDat interface
//
#define EnDat_Write(base_addr, reg, data) (IOWR_32DIRECT(base_addr, reg*4, data))

//
// Low level read function for EnDat interface
//
#define EnDat_Read(base_addr, reg) (IORD_32DIRECT(base_addr, reg*4))

/*!
 * @}
 */

/*!
 * @}
 */

#endif /* ENDAT_API_H_ */
